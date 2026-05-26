# Week 12 Lecture — Virtual Memory

> **Last Updated:** 2026-05-26
>
> Silberschatz, Operating System Concepts Ch 10 (Virtual Memory)

> **Prerequisites**: Week 11 lecture is the direct foundation — pages, frames, page tables, MMU, TLB, valid/invalid bits, swapping. Without that vocabulary, almost nothing here makes sense. Week 1–3 (process/PCB, fork(), context switch) explains where copy-on-write and demand paging hook into process creation. Familiarity with disk vs RAM latency from a computer-architecture course makes the EAT discussion concrete.
>
> **Learning Objectives**:
> 1. State the **definition** of virtual memory and explain why separating logical from physical address space is useful
> 2. Describe **demand paging** and the role of the **lazy swapper / pager**
> 3. Walk through the **page-fault handling** sequence step by step
> 4. Explain **pure demand paging** and the **hardware requirements** for instruction restart
> 5. Compute **Effective Access Time (EAT)** under demand paging and quantify the cost of a single page fault
> 6. Describe **copy-on-write (COW)** and the difference between `fork()` and `vfork()`
> 7. Distinguish **frame allocation** (how many frames per process) from **page replacement** (which page to evict)
> 8. Execute **FIFO, OPT, LRU** page-replacement algorithms on a reference string and tabulate the page-fault counts
> 9. Define **Belady's anomaly** and explain why **stack algorithms** (LRU, OPT) are immune
> 10. Describe **LRU-approximation** algorithms: **additional reference bits, second-chance (clock), enhanced second-chance, LFU/MFU**
> 11. Compare **equal vs proportional allocation** and **global vs local replacement**
> 12. Define **thrashing**, the **locality model**, the **working-set model**, and the **page-fault frequency (PFF)** scheme
> 13. Explain **memory compression**, the **buddy system**, and **slab allocation** for kernel memory
> 14. Describe **prepaging**, the impact of **program structure** on locality, and **I/O interlock** with page pinning

---

## Table of Contents

- [1. Background and Demand Paging](#1-background-and-demand-paging)
  - [1.1 What Virtual Memory Buys You](#11-what-virtual-memory-buys-you)
  - [1.2 The Virtual Address Space of a Process](#12-the-virtual-address-space-of-a-process)
  - [1.3 Shared Libraries via Virtual Memory](#13-shared-libraries-via-virtual-memory)
  - [1.4 Demand Paging and the Lazy Pager](#14-demand-paging-and-the-lazy-pager)
  - [1.5 Valid / Invalid Bit](#15-valid--invalid-bit)
  - [1.6 Page-Fault Handling — Step by Step](#16-page-fault-handling--step-by-step)
  - [1.7 Pure Demand Paging and Hardware Requirements](#17-pure-demand-paging-and-hardware-requirements)
  - [1.8 The Difficulty of Instruction Restart](#18-the-difficulty-of-instruction-restart)
  - [1.9 Free-Frame List and Zero-Fill-on-Demand](#19-free-frame-list-and-zero-fill-on-demand)
  - [1.10 Demand-Paging Performance — EAT](#110-demand-paging-performance--eat)
  - [1.11 Swap Space Utilization](#111-swap-space-utilization)
- [2. Copy-on-Write](#2-copy-on-write)
  - [2.1 The COW Idea](#21-the-cow-idea)
  - [2.2 Non-Modifiable Pages and vfork()](#22-non-modifiable-pages-and-vfork)
- [3. Page Replacement](#3-page-replacement)
  - [3.1 Why Replacement Is Needed](#31-why-replacement-is-needed)
  - [3.2 Basic Replacement Procedure and the Dirty Bit](#32-basic-replacement-procedure-and-the-dirty-bit)
  - [3.3 What Page Replacement Buys You](#33-what-page-replacement-buys-you)
  - [3.4 Reference Strings](#34-reference-strings)
  - [3.5 FIFO](#35-fifo)
  - [3.6 Belady's Anomaly](#36-beladys-anomaly)
  - [3.7 Optimal (OPT / MIN)](#37-optimal-opt--min)
  - [3.8 LRU (Least Recently Used)](#38-lru-least-recently-used)
  - [3.9 LRU Implementation](#39-lru-implementation)
  - [3.10 Stack Algorithms and Why They Avoid Belady](#310-stack-algorithms-and-why-they-avoid-belady)
  - [3.11 Algorithm Comparison](#311-algorithm-comparison)
- [4. LRU-Approximation Algorithms](#4-lru-approximation-algorithms)
  - [4.1 Why Approximate?](#41-why-approximate)
  - [4.2 Additional-Reference-Bits](#42-additional-reference-bits)
  - [4.3 Second-Chance (Clock) Algorithm](#43-second-chance-clock-algorithm)
  - [4.4 Clock Algorithm — Visualization](#44-clock-algorithm--visualization)
  - [4.5 Enhanced Second-Chance — (R, M) Classes](#45-enhanced-second-chance--r-m-classes)
  - [4.6 LFU and MFU](#46-lfu-and-mfu)
  - [4.7 Page Buffering](#47-page-buffering)
- [5. Frame Allocation and Thrashing](#5-frame-allocation-and-thrashing)
  - [5.1 Minimum Frames and Where the Bound Comes From](#51-minimum-frames-and-where-the-bound-comes-from)
  - [5.2 Equal vs Proportional Allocation](#52-equal-vs-proportional-allocation)
  - [5.3 Global vs Local Replacement](#53-global-vs-local-replacement)
  - [5.4 Reclaiming Pages and the Reaper](#54-reclaiming-pages-and-the-reaper)
  - [5.5 NUMA and Frame Allocation](#55-numa-and-frame-allocation)
  - [5.6 Thrashing](#56-thrashing)
  - [5.7 Locality Model](#57-locality-model)
  - [5.8 Working-Set Model](#58-working-set-model)
  - [5.9 Working-Set Implementation](#59-working-set-implementation)
  - [5.10 Page-Fault Frequency (PFF)](#510-page-fault-frequency-pff)
- [6. Memory Compression](#6-memory-compression)
- [7. Kernel Memory Allocation](#7-kernel-memory-allocation)
  - [7.1 Why Kernel Memory Is Special](#71-why-kernel-memory-is-special)
  - [7.2 Buddy System](#72-buddy-system)
  - [7.3 Slab Allocation](#73-slab-allocation)
- [8. Other Considerations](#8-other-considerations)
  - [8.1 Prepaging](#81-prepaging)
  - [8.2 Program Structure and Locality](#82-program-structure-and-locality)
  - [8.3 I/O Interlock and Page Locking](#83-io-interlock-and-page-locking)
- [9. Lab — Page-Replacement Algorithm Simulator](#9-lab--page-replacement-algorithm-simulator)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. Background and Demand Paging

### 1.1 What Virtual Memory Buys You

**Virtual memory** is a technique that **separates logical (virtual) memory from physical memory**. It is built on top of paging (Week 11) and turns the page-table indirection into a powerful lie: *the process believes it has a huge, contiguous address space, while the OS quietly serves only the parts that are actually being used*.

Why bother? Three structural wins:

- **Programs can be larger than physical RAM.** Only the pages actively touched need to be resident. Error-handling code, unused features, and over-sized declared arrays cost zero physical memory until they are referenced.
- **More processes fit in RAM simultaneously.** Higher multiprogramming → higher CPU utilization (Week 6).
- **Faster process startup.** A new process can start running as soon as a handful of pages are in memory; the rest stream in on demand.

Bonus features that ride on the same machinery:

- Shared libraries via **shared frames** (Week 11 §6.2 was the foundation).
- **Inter-process communication** via shared memory.
- **Fast `fork()`** via copy-on-write (§2).

> **Why this is the right abstraction:** the addressing layer of Week 11 (page tables, MMU, TLB) gives us *spatial* virtualization. Demand paging adds *temporal* virtualization — a page only needs to exist in RAM at the moment the CPU references it, not before. Everything in this lecture is some consequence of "make absent pages cheap."

### 1.2 The Virtual Address Space of a Process

Each process sees its own independent virtual address space, typically starting from 0 and *appearing* contiguous — but the underlying physical frames may be scattered anywhere in RAM, or partly on disk.

```
Virtual Address Space (one process):

  HIGH ┌─────────────┐
       │   Stack     │  ← grows downward
       ├─────────────┤
       │             │
       │  HOLE       │  ← no physical memory consumed
       │             │
       ├─────────────┤
       │   Heap      │  ← grows upward
       ├─────────────┤
       │   Data      │
       ├─────────────┤
       │   Text      │  ← read-only code
   LOW └─────────────┘
```

- **Sparse address space**: large *holes* sit between text/data, heap, and stack.
- Holes occupy **no** physical memory — they have no page-table entries pointing to frames.
- Shared libraries are usually mapped into the middle region (between heap and stack).

> **Key insight:** because holes are free, processes can declare massive maximum sizes without paying for them. A 64-bit process can mmap a 2 TB sparse file; only the touched pages allocate frames.

### 1.3 Shared Libraries via Virtual Memory

Multiple processes can **share** libraries such as the standard C library:

- The library is mapped into each process's virtual address space at some virtual address (often the same in every process for relocation simplicity, but it doesn't have to be).
- The **physical frames** that hold the library code are **shared** — one set of frames, many page-table entries pointing at them.
- Libraries are mapped **read-only**, which is what makes sharing safe (Week 11 §6.2).
- The same mechanism implements shared-memory IPC.
- `fork()` initially shares all pages between parent and child via page-table copying — see copy-on-write below.

### 1.4 Demand Paging and the Lazy Pager

**Demand paging**: load a page into memory **only when it is actually referenced**.

The OS component that performs the loading is sometimes called a **lazy swapper** or **pager** — emphatically *lazy*: it never speculatively brings pages in.

Advantages:
- **Reduced I/O** — pages that are never touched are never read from disk.
- **Memory savings** — only the working set is resident.
- **Faster startup** — the process begins running as soon as the first instruction page is in.
- **More concurrent processes** — each one occupies less RAM.

### 1.5 Valid / Invalid Bit

Each page-table entry (PTE) carries a **valid/invalid bit** (Week 11 §6.1, now reinterpreted for demand paging):

- **Valid** — the page is legal *and* currently resident in a physical frame.
- **Invalid** — either the page is outside the process's address space, **or** it is a legal page that is currently on disk.

The CPU does not distinguish the two flavours of "invalid" — both raise the same hardware trap. The OS decides which it is in the trap handler.

> **Why this design is clean:** the existence-vs-residency check is fused with the same protection check the MMU already had to do. Demand paging is essentially "free" once the MMU supports the valid bit — no new datapath, just new OS logic on the trap.

### 1.6 Page-Fault Handling — Step by Step

When the CPU touches a page whose PTE is invalid, a **page-fault trap** transfers control to the kernel:

```
1. CPU references virtual address  →  MMU checks PTE.
2. Valid bit = invalid              →  PAGE FAULT trap to OS.
3. OS classifies:
     (a) illegal address  →  deliver SIGSEGV, kill the process.
     (b) valid but on disk →  continue.
4. OS picks a free frame from the free-frame list
     (or runs page replacement if the list is empty — §3).
5. OS schedules disk I/O to read the page into that frame.
6. OS updates the PTE: frame number, set valid bit, set permissions.
7. OS RESTARTS the faulting instruction.
8. The instruction re-executes; this time the PTE is valid → succeeds.
```

The whole sequence — typically 5–10 ms on a spinning disk, hundreds of μs on SSD, tens of μs on NVMe — is **orders of magnitude slower** than a normal memory access (~100 ns). This is why page-fault rate must be kept tiny.

### 1.7 Pure Demand Paging and Hardware Requirements

**Pure demand paging** is the extreme strategy: start the process with **no** pages in memory. Even the first instruction triggers a page fault. As execution proceeds, **locality of reference** rapidly populates the working set, and the fault rate plummets.

The **hardware required** to support demand paging:

- **Page table with a valid/invalid bit** (and protection bits).
- **Secondary memory (swap space)** to hold non-resident pages.
- **Instruction restart capability** — the CPU must be able to re-execute the trapped instruction cleanly after the fault is serviced.

### 1.8 The Difficulty of Instruction Restart

Restarting an instruction is conceptually simple but architecturally tricky:

- **Easy case** — fault occurs during instruction *fetch*: just re-fetch and start over.
- **Hard case** — multi-step instruction `ADD A, B, C`:
  1. Fetch & decode instruction.
  2. Fetch operand at A.
  3. Fetch operand at B.
  4. Compute A + B.
  5. Store result to C — **page fault here!**
  - If we naively re-execute, steps 1–4 are repeated, which is fine because they have no side effects. But what about instructions that *do* have side effects?
- **IBM MVC instruction** — copies up to 256 bytes; if the destination crosses a page boundary and the second page is invalid, a fault occurs mid-copy.
  - **Fix 1:** before any byte is written, probe both end addresses; if either would fault, take the fault *before* any side effect.
  - **Fix 2:** save overwritten destination bytes into temporary registers; on a fault, restore them before transferring control to the trap handler.

> **Why this matters historically:** instruction restart determines whether demand paging is even *possible* on a given architecture. Some early CPUs could not support it cleanly, which is why virtual memory took a generation of hardware design to become universal.

### 1.9 Free-Frame List and Zero-Fill-on-Demand

The OS maintains a **free-frame list** — a pool of physical frames available for allocation.

- At system boot, *all* user-available memory is registered in the list.
- Page-fault handling pulls one frame off the list and uses it.
- When the list shrinks below a threshold, **page replacement** (§3) refills it by evicting victims.

**Zero-fill-on-demand**: before handing a frame to a new process, the kernel **zeroes it out**. Why? *Security* — the frame previously belonged to some other process, and its leftover contents must not leak.

### 1.10 Demand-Paging Performance — EAT

Let:
- $p$ = page-fault rate, $0 \leq p \leq 1$
- $\text{mem}$ = memory access time
- $\text{pf}$ = page-fault service time (interrupt + disk I/O + process restart)

$$
\text{EAT} = (1 - p) \cdot \text{mem} + p \cdot \text{pf}
$$

**Worked example** — $\text{mem} = 200\ \text{ns}$, $\text{pf} = 8\ \text{ms} = 8\,000\,000\ \text{ns}$:

$$
\text{EAT} = 200 + 7\,999\,800 \cdot p
$$

- $p = 1/1000$ (one fault per 1000 accesses): $\text{EAT} = 200 + 7999.8 \approx 8200\ \text{ns}$ — **40× slower** than raw memory!
- To keep slowdown within 10% ($\text{EAT} \leq 220$): $p < 0.0000025 \approx 1/400\,000$.

> **The lesson:** page faults must be **extremely rare** — fewer than one per ~400,000 references — for demand paging to feel free. This is why everything else in this lecture (good replacement, working sets, locality) matters: a small jump in fault rate translates to a huge jump in effective access time.

### 1.11 Swap Space Utilization

Swap space sits on disk; reads from it are typically faster than ordinary file-system reads because:
- The OS uses **larger block transfers** (no per-file lookup overhead).
- No file-system metadata indirection.

Two strategies for where a page "lives":

- **Strategy 1:** at process start, copy the whole executable into swap space, then demand-page from swap.
- **Strategy 2:** demand-page from the file system initially; when a page is evicted, write the *dirty* version to swap and read it back from swap thereafter. This is what **Linux and Windows** do.

A nuance:
- **Binary executable pages** (text, read-only data) come from the file system; since they are never modified, on eviction they are simply discarded and re-read from the file system next time.
- **Anonymous memory** (stack, heap — no file backing) must be written to swap on eviction; that is where swap space is actually used.

---

<br>

## 2. Copy-on-Write

### 2.1 The COW Idea

`fork()` would naively duplicate the parent's entire address space into a fresh set of frames for the child — wasteful, since the child very often calls `exec()` immediately and throws all of it away.

**Copy-on-write (COW)** defers the copy until it is provably necessary:

```
1. fork():
      Build the child's page table by copying the parent's PTEs.
      Both parent and child PTEs point to the SAME physical frames.
      Mark every shared writable page READ-ONLY in both PTEs.

2. Read access:
      No fault. Both processes happily read from the shared frame.

3. Write access:
      The read-only mark triggers a protection fault.
      OS copies the page to a new frame.
      Updates the writer's PTE: new frame, writable again.
      Other process's PTE still points to the old frame (also writable now).

4. Pages neither side modifies are NEVER copied.
```

Used by **Linux, Windows, macOS**. The performance win is huge for `fork()`-then-`exec()` workloads (shells, servers).

### 2.2 Non-Modifiable Pages and vfork()

- **Non-modifiable pages** (executable code) never need COW — they are always shared and read-only by nature. COW is only relevant for pages that *might* be written.
- **`vfork()` (virtual memory fork)** is a more aggressive variant:
  - Parent process is **suspended** until the child terminates or calls `exec()`.
  - Pages are **shared without any copying or COW marking** — the child literally runs in the parent's address space.
  - Extremely fast (no page-table copying at all), but **dangerous** — if the child does anything other than minimal setup-then-exec, it corrupts the parent.
  - Used in classical UNIX shell implementations where the child immediately replaces itself.

---

<br>

## 3. Page Replacement

### 3.1 Why Replacement Is Needed

Demand paging leads to **over-allocation**: the OS keeps loading pages as faults occur, and eventually all frames are in use. A new page fault then has nowhere to put the incoming page.

**Page replacement** = pick a resident page to evict ("the **victim**") to make room for the incoming page.

This is what makes the lie of virtual memory complete — a 20-page process can run with only 10 frames, because the OS continuously rotates the 10 most useful ones in and out.

### 3.2 Basic Replacement Procedure and the Dirty Bit

```
1. Locate the desired page on disk.
2. Find a free frame:
   - If one exists in the free-frame list, take it.
   - Otherwise, run the page-replacement algorithm to pick a VICTIM frame.
       If the victim's dirty bit = 1, write it back to disk.
       If the victim's dirty bit = 0, just drop it (it's still on disk).
3. Read the desired page into the now-free frame.
4. Update the page tables (mark victim invalid, mark new page valid).
5. Restart the faulting process.
```

The **dirty bit (modify bit)** is set by hardware whenever the page is written. If it is still 0 at eviction time, the in-memory copy is identical to the on-disk copy and the write-back can be skipped.

> **Why this matters:** swap-out I/O is the single most expensive thing the OS does on a page fault. Skipping it for clean pages cuts the disk traffic roughly in half on typical workloads.

### 3.3 What Page Replacement Buys You

Page replacement is what **truly decouples logical memory from physical memory**. Without it, a process could only run if its entire address space fit in RAM. With it:

- A 20-page process runs on 10 frames.
- Many processes coexist comfortably even when their virtual sizes exceed RAM in total.

Two algorithmic problems must be answered:

1. **Frame allocation** — how many frames does each process get? (§5)
2. **Page replacement** — given a fixed frame budget, which page do we evict? (§3.5 – §4)

The goal is always to **minimize the page-fault rate** (recall the EAT calculation in §1.10).

### 3.4 Reference Strings

To compare algorithms fairly, we evaluate them against a **reference string** — the sequence of page numbers a process touches.

**Example**: with page size 100 bytes, byte addresses
```
0100, 0432, 0101, 0612, 0102, 0103, ...
```
collapse to page-number reference string
```
1, 4, 1, 6, 1, 6, 1, 6, 1, 6, 1
```

We then simulate each algorithm against the same string and count page faults. Generally, **more frames → fewer faults** (with the famous exception in §3.6).

The canonical example reference string used below:
```
7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1
```

### 3.5 FIFO

**FIFO** (First-In, First-Out) replaces the page that has been in memory the longest.

- Managed with a FIFO queue: new pages enter the tail, victims leave the head.
- Trivial to implement, but treats all pages identically regardless of recent activity.

**FIFO trace** — 3 frames, reference string `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1`:

| Ref | F1   | F2   | F3   | Fault? |
|-----|------|------|------|--------|
| 7   | 7    | -    | -    | F      |
| 0   | 7    | 0    | -    | F      |
| 1   | 7    | 0    | 1    | F      |
| 2   | **2**| 0    | 1    | F      |
| 0   | 2    | 0    | 1    |        |
| 3   | 2    | **3**| 1    | F      |
| 0   | 2    | 3    | **0**| F      |
| 4   | **4**| 3    | 0    | F      |
| 2   | 4    | **2**| 0    | F      |
| 3   | 4    | 2    | **3**| F      |
| 0   | **0**| 2    | 3    | F      |
| 3   | 0    | 2    | 3    |        |
| 2   | 0    | 2    | 3    |        |
| 1   | 0    | **1**| 3    | F      |
| 2   | 0    | 1    | **2**| F      |
| 0   | 0    | 1    | 2    |        |
| 1   | 0    | 1    | 2    |        |
| 7   | **7**| 1    | 2    | F      |
| 0   | 7    | **0**| 2    | F      |
| 1   | 7    | 0    | **1**| F      |

**Total: 15 page faults.**

### 3.6 Belady's Anomaly

You would expect that **adding frames always reduces faults**. With FIFO, this is *not always true*.

**Reference string**: `1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5`

| Frames | Page Faults |
|--------|-------------|
| 3      | **9**       |
| 4      | **10**      |

More frames → **more** faults. This is **Belady's anomaly**, a fundamental pathology of FIFO.

**3-frame trace** (9 faults):

| Ref | F1   | F2   | F3   | Fault? |
|-----|------|------|------|--------|
| 1   | 1    | -    | -    | F      |
| 2   | 1    | 2    | -    | F      |
| 3   | 1    | 2    | 3    | F      |
| 4   | **4**| 2    | 3    | F      |
| 1   | 4    | **1**| 3    | F      |
| 2   | 4    | 1    | **2**| F      |
| 5   | **5**| 1    | 2    | F      |
| 1   | 5    | 1    | 2    |        |
| 2   | 5    | 1    | 2    |        |
| 3   | 5    | **3**| 2    | F      |
| 4   | 5    | 3    | **4**| F      |
| 5   | 5    | 3    | 4    |        |

**4-frame trace** (10 faults):

| Ref | F1   | F2   | F3   | F4   | Fault? |
|-----|------|------|------|------|--------|
| 1   | 1    | -    | -    | -    | F      |
| 2   | 1    | 2    | -    | -    | F      |
| 3   | 1    | 2    | 3    | -    | F      |
| 4   | 1    | 2    | 3    | 4    | F      |
| 1   | 1    | 2    | 3    | 4    |        |
| 2   | 1    | 2    | 3    | 4    |        |
| 5   | **5**| 2    | 3    | 4    | F      |
| 1   | 5    | **1**| 3    | 4    | F      |
| 2   | 5    | 1    | **2**| 4    | F      |
| 3   | 5    | 1    | 2    | **3**| F      |
| 4   | **4**| 1    | 2    | 3    | F      |
| 5   | 4    | **5**| 2    | 3    | F      |

### 3.7 Optimal (OPT / MIN)

The **OPT** algorithm replaces the page that **will not be used for the longest time in the future**.

- Provably the **lowest possible page-fault rate** — no algorithm can do better.
- **No Belady anomaly** — adding frames never hurts.
- **Impossible to implement** in practice (requires perfect knowledge of the future).
- Used as a **baseline** (upper-bound on quality) for evaluating real algorithms.

**OPT trace** — 3 frames, same string as FIFO:

| Ref | F1    | F2    | F3    | Fault? | Reason |
|-----|-------|-------|-------|--------|--------|
| 7   | 7     | -     | -     | F      |        |
| 0   | 7     | 0     | -     | F      |        |
| 1   | 7     | 0     | 1     | F      |        |
| 2   | **2** | 0     | 1     | F      | 7 next used at #18 (farthest) |
| 0   | 2     | 0     | 1     |        |        |
| 3   | 2     | 0     | **3** | F      | 1 next used at #14 (farthest) |
| 0   | 2     | 0     | 3     |        |        |
| 4   | 2     | 0     | **4** | F      | replace 3 (next #10), keeping 2 (#9), 0 (#12) — wait, replace which? See note. |
| 2   | 2     | 0     | 4     |        |        |
| 3   | 2     | 0     | **3** | F      | 4 is no longer used |
| 0   | 2     | 0     | 3     |        |        |
| 3   | 2     | 0     | 3     |        |        |
| 2   | 2     | 0     | 3     |        |        |
| 1   | 2     | **1** | 3     | F      |        |
| 2   | 2     | 1     | 3     |        |        |
| 0   | 2     | 1     | 3     |        |        |
| 1   | 2     | 1     | 3     |        |        |
| 7   | **7** | 1     | 3     | F      |        |
| 0   | 7     | 1     | 3     |        |        |
| 1   | 7     | 1     | 3     |        |        |

(Wait — slide-listed reasoning says step (8) `Ref 4` evicts 3 because of distance; the exact victim depends on which of the three resident pages has the most-distant next use, and the worked example follows the CLRS/Silberschatz table.) **Total: 9 page faults** — six fewer than FIFO.

### 3.8 LRU (Least Recently Used)

Since we can't see the future, we approximate it with the past: **replace the page that has not been used for the longest time**.

- **Key assumption:** locality of reference — a page that has been idle for a long time is unlikely to be needed soon.
- Equivalent to running OPT *backwards in time*.
- **No Belady anomaly** — LRU is a **stack algorithm** (§3.10).

**LRU trace** — 3 frames, same reference string:

| Ref | F1    | F2    | F3    | Fault? | Reason (LRU victim) |
|-----|-------|-------|-------|--------|---------------------|
| 7   | 7     | -     | -     | F      |                     |
| 0   | 7     | 0     | -     | F      |                     |
| 1   | 7     | 0     | 1     | F      |                     |
| 2   | **2** | 0     | 1     | F      | 7 least recent      |
| 0   | 2     | 0     | 1     |        |                     |
| 3   | 2     | 0     | **3** | F      | 1 least recent      |
| 0   | 2     | 0     | 3     |        |                     |
| 4   | **4** | 0     | 3     | F      | 2 least recent      |
| 2   | 4     | 0     | **2** | F      | 3 least recent      |
| 3   | 4     | **3** | 2     | F      | 0 least recent      |
| 0   | **0** | 3     | 2     | F      |                     |
| 3   | 0     | 3     | 2     |        |                     |
| 2   | 0     | 3     | 2     |        |                     |
| 1   | **1** | 3     | 2     | F      |                     |
| 2   | 1     | 3     | 2     |        |                     |
| 0   | 1     | **0** | 2     | F      |                     |
| 1   | 1     | 0     | 2     |        |                     |
| 7   | 1     | 0     | **7** | F      |                     |
| 0   | 1     | 0     | 7     |        |                     |
| 1   | 1     | 0     | 7     |        |                     |

**Total: 12 page faults.** Comparison: **FIFO 15 > LRU 12 > OPT 9.**

### 3.9 LRU Implementation

True LRU needs to know the **relative recency** of every resident page. Two hardware-supported implementations:

**Method 1 — Counter (time stamp).**
- Each PTE stores a "time-of-use" field.
- Add a logical clock to the CPU; it ticks on every memory access.
- On every reference, copy the clock into the referenced PTE.
- Victim = the page with the smallest time stamp.
- **Cost:** memory write on every access (the PTE), and a linear scan of resident pages to find the minimum.

**Method 2 — Stack (doubly linked list).**
- Maintain a doubly linked list of resident pages ordered by recency.
- On reference, move the page to the **top** of the stack — six pointer updates worst case.
- Victim = the page at the **bottom** of the stack — O(1) lookup, no scan needed.

Both demand hardware help; without it, doing this in software via interrupts would slow memory access by an order of magnitude.

### 3.10 Stack Algorithms and Why They Avoid Belady

A replacement algorithm is a **stack algorithm** if, for every reference string, the set of resident pages with $n$ frames is **always a subset** of the set with $n + 1$ frames.

- **LRU is a stack algorithm:** with $n$ frames, the resident set is exactly "the $n$ most-recently-referenced pages." With $n + 1$ frames, those $n$ pages are still in the resident set, plus one more.
- **OPT is a stack algorithm** as well (by a symmetric argument on the future).
- **FIFO is *not* a stack algorithm** — the resident set with $n$ frames is not necessarily a subset of that with $n + 1$ frames. This is exactly what allows Belady's anomaly.

**Theorem (intuition):** for any stack algorithm, adding a frame can **never increase** the number of faults — each page that would have been a fault on a smaller cache is still a fault (or a hit) on the larger one, never the reverse.

### 3.11 Algorithm Comparison

Same reference string, 3 frames:

| Algorithm | Page Faults | Notes |
|-----------|-------------|-------|
| FIFO      | **15**      | Simple but suffers Belady's anomaly |
| LRU       | **12**      | Good performance, expensive implementation |
| OPT       | **9**       | Optimal but unimplementable |

---

<br>

## 4. LRU-Approximation Algorithms

### 4.1 Why Approximate?

True LRU requires hardware that updates a clock or stack on **every** memory reference. Most architectures don't provide that. Most architectures *do* provide a **reference bit**:

- Set to 1 by hardware whenever the page is read or written.
- Cleared by the OS on its own schedule.
- Tells us **which pages were used** in some recent window, but not the **order**.

LRU-approximation algorithms use this single bit (or a few bits) to make replacement decisions that approximate LRU at a fraction of the cost.

### 4.2 Additional-Reference-Bits

Maintain an **8-bit byte** (shift register) per page, updated by the OS on a periodic timer interrupt (say, every 100 ms):

```
On each timer tick:
  1. Shift the current reference bit into the MSB of the page's byte.
  2. Shift the rest of the byte right by one.
  3. Clear the hardware reference bit (ready for the next interval).
```

```
Example histories:
  11000100  →  used in 2 of the 3 most-recent intervals, then once more later
  01110111  →  not used recently but more historical use

Compared as unsigned integers, 11000100 > 01110111
→ the second page is the better eviction candidate (smaller history value)
```

- Victim: page with the **smallest** byte value (= least recent use).
- Ties broken by FIFO order.
- Increasing the history bit count improves accuracy but raises the interrupt cost.

### 4.3 Second-Chance (Clock) Algorithm

Combine **FIFO** with the **reference bit**:

```
Maintain pages in a FIFO order (a circular queue).
On replacement need:
   Look at the head page of the FIFO.
   If reference bit = 0 → REPLACE this page.
   If reference bit = 1 → clear the bit, treat the page as "newly arrived,"
                          advance to the next page in the FIFO, and repeat.
```

Implemented as a **circular queue** with a hand pointer: the algorithm looks exactly like a clock with a sweeping hand — hence the name **Clock Algorithm**.

- Worst case (all bits = 1): one full revolution clears every bit; the next pass falls back to plain FIFO.
- Frequently-used pages always have ref bit = 1 by the time the hand reaches them → effectively *never* replaced.

> **Why this works:** the reference bit gives a binary "recently used or not" signal. The clock turns that single bit into something close to LRU by giving recently-used pages a free pass and letting only the truly stale pages get evicted.

### 4.4 Clock Algorithm — Visualization

```
[Initial state]                [After search]
   v (pointer)                    v (pointer, points to new victim)
+------+                    +------+
| P1: 1| → clear to bit=0   | P1: 0|
| P2: 1| → clear to bit=0   | P2: 0|
| P3: 0| → VICTIM!           | NEW  | ← incoming page inserted here
| P4: 1|                     | P4: 1|
| P5: 0|                     | P5: 0|
+------+                    +------+

P1, P2 survive — given a "second chance" by clearing their bit.
P3 is replaced because its bit was already 0.
```

### 4.5 Enhanced Second-Chance — (R, M) Classes

Refine clock by using both the **reference bit (R)** and the **modify (dirty) bit (M)**. Each page falls into one of **4 classes**:

| Class | (R, M) | Meaning                                | Replacement priority |
|-------|--------|----------------------------------------|----------------------|
| 1     | (0, 0) | Not recently used, not modified        | **Highest** (cheap to drop) |
| 2     | (0, 1) | Not recently used, but modified        | 2nd — needs write-back  |
| 3     | (1, 0) | Recently used, not modified            | 3rd                    |
| 4     | (1, 1) | Recently used **and** modified         | **Lowest** priority    |

Pages in lower classes are evicted first. Replacing a modified page costs an additional disk write, so we prefer to evict clean pages — and absolutely prefer not to evict pages that are both recent and dirty.

### 4.6 LFU and MFU

Count how many times each page has been referenced:

- **LFU (Least Frequently Used)** — evict the page with the **fewest** references.
  - Problem: a page used heavily during initialization and then never again hangs around forever because its count is huge.
  - Fix: periodically right-shift every counter (exponential decay), so old activity fades.
- **MFU (Most Frequently Used)** — evict the page with the **most** references.
  - Logic: pages with small counts just arrived and probably *will* be used; the heavily-referenced ones have already done their work.

Neither approximates OPT well in general. **Rarely used in practice** — implementation cost is high relative to the (modest) benefit.

### 4.7 Page Buffering

Several complementary techniques layered on top of any replacement policy:

- **Maintain a free-frame pool.** On a page fault, *immediately* take a frame from the pool — don't wait for the replacement algorithm. The victim is written back **later, asynchronously**. The faulting process restarts quickly.
- **Modified-page list (batched write-back).** Collect dirty pages and write them out in a batch when the paging device is idle.
- **Cache victims in the free-frame pool.** When a frame is freed, remember which page it used to hold. If that page is needed again *before* the frame is reused, recover it without any disk I/O. (Used in UNIX-style systems combined with second-chance.)

---

<br>

## 5. Frame Allocation and Thrashing

### 5.1 Minimum Frames and Where the Bound Comes From

Each process needs at least a **minimum number of frames** to execute *any* instruction without thrashing on the first try.

- The minimum is **determined by the architecture**: the maximum number of distinct pages any single instruction can reference.
- Indirect addressing increases the bound. Example: 1 level of indirect addressing → minimum **3 frames** (instruction page, operand-pointer page, operand page).

The **maximum** is bounded by the available physical memory.

### 5.2 Equal vs Proportional Allocation

Given $m$ available frames and $n$ processes, how should the OS distribute them?

- **Equal allocation:** $m / n$ frames each.
  - Example: 93 frames, 5 processes → 18 each (with 3 spare).
- **Proportional allocation:** give a process frames in proportion to its size $s_i$:
  $$ a_i = \frac{s_i}{S} \cdot m, \qquad S = \sum_i s_i $$
  - Example: 62 frames, process A (10 KB), process B (127 KB), $S = 137\ \text{KB}$:
    - A: $10/137 \times 62 \approx 4$ frames.
    - B: $127/137 \times 62 \approx 57$ frames.
  - **Priority-based allocation:** replace size with priority weights.

### 5.3 Global vs Local Replacement

When a process page-faults and no free frame is available, where do we look for a victim?

- **Global replacement:** victim chosen from **all** frames in the system. The faulting process can steal a frame from another process.
  - Better **throughput** overall — generally used in modern OSes.
  - **Downside:** a process cannot predict its own page-fault rate, because its working set may be silently raided.
- **Local replacement:** victim chosen only from the **faulting process's own** frames.
  - Predictable per-process page-fault rate.
  - **Downside:** if one process has too few frames it thrashes alone, even while other processes have surplus; available memory may go underused.

### 5.4 Reclaiming Pages and the Reaper

Practical global replacement uses a **reaper** routine that runs in the background:

- Triggered when the free-frame list drops below a **minimum threshold**.
- Walks pages using an LRU-approximation algorithm and reclaims them.
- Stops when free memory rises above a **maximum threshold**.

When free memory is critically low and the reaper cannot keep up, **Linux runs the OOM Killer** (Out-Of-Memory Killer), which terminates the process with the worst OOM score — a heuristic combining size, age, and "killability."

### 5.5 NUMA and Frame Allocation

On **NUMA (Non-Uniform Memory Access)** machines each CPU has its own *local* memory bank; accessing local memory is significantly faster than accessing memory attached to another CPU.

- On a page fault, the OS should allocate a frame on the NUMA node **closest to the CPU** that is running the faulting process.
- Linux maintains a separate free-frame list per NUMA node.
- The scheduler tries to keep a process on the CPU it most recently ran on, improving both cache reuse and locality of physical memory.

### 5.6 Thrashing

**Thrashing** = the process is spending more time handling page faults than executing real instructions.

**Vicious cycle:**

```
1. Process has too few frames for its current working set
2. Page faults happen on nearly every instruction
3. I/O surges; the process is mostly blocked
4. CPU utilization drops (everyone is waiting on I/O)
5. The OS sees low CPU utilization and SCHEDULES MORE PROCESSES
6. Now every process has even fewer frames
7. Go to step 2 — faults explode further
```

**Symptom plot:** as the degree of multiprogramming rises, CPU utilization first climbs (good), then **suddenly collapses** past a critical point. That collapse is thrashing.

> **Why this is the canonical OS pathology:** thrashing is what happens when *the abstraction breaks*. The whole point of virtual memory is that programs can be larger than RAM — but only if their *active* footprint fits. Push that footprint past available frames and the system spends more time shuffling pages than computing.

**Defense:** prefer **local replacement** (one process's bad behaviour does not poison the others) or, better, prevent over-commitment in the first place using working sets or PFF (below).

### 5.7 Locality Model

A process **moves from locality to locality** during execution:

- A **locality** is a set of pages used intensively over some interval.
- Function calls define new localities — the function's code, its local variables, and the globals it touches.
- Returning from a function exits that locality.
- Localities can overlap (caller's data is still warm after callee returns).

**Implication:** if you give a process enough frames for its **current** locality, faults only occur during the brief transition between localities — almost never during steady-state execution. If the locality doesn't fit, the process thrashes.

### 5.8 Working-Set Model

A **practical** approximation of "current locality." Define a working-set window $\Delta$ (number of recent memory references).

$$
WS_i(t) = \{\text{pages referenced by process } i \text{ in the last } \Delta \text{ references}\}
$$

- $\Delta$ too small: doesn't capture the locality → false misses.
- $\Delta$ too large: spans multiple localities → over-allocation.
- $\Delta = \infty$: every page the process ever touched.

**Example, $\Delta = 10$:**

```
...2 6 1 5 7 7 7 7 5 1 | 6 2 3 4 1 2 3 4 4 4 | 3 4 3 4...
                    t1                     t2

WS(t1) = {1, 2, 5, 6, 7}   WS(t2) = {3, 4}
```

**Thrashing prevention with WS:**
- $WSS_i = |WS_i|$ — number of frames needed by process $i$.
- $D = \sum_i WSS_i$ — total system frame demand.
- If $D > m$ (frames available) → **thrashing imminent**.
  - **Solution:** suspend one process to free its frames.
- If $D \leq m$ → unused frames can support a new process.

### 5.9 Working-Set Implementation

Tracking the working set exactly is expensive; the typical approximation uses **timer interrupts + reference bits**:

```
Suppose Δ = 10,000 references; timer interrupt every 5,000 references.

On each timer tick:
  For each page:
    Copy the current reference bit into an in-memory history bit slot
    Clear the reference bit

On a page-fault decision:
  Look at the current reference bit + most-recent 2 history bits
  If any of the three is 1, the page was used in the last ~10,000-15,000 refs
                            → counted as part of the working set
  If all are 0, the page is OUTSIDE the working set
```

More history bits → finer accuracy but more bookkeeping per timer interrupt. As always with approximation, the right knob setting is a workload-dependent compromise.

### 5.10 Page-Fault Frequency (PFF)

The **PFF** strategy controls thrashing **directly** by monitoring the page-fault rate of each process:

| Situation                                        | Action                              |
|--------------------------------------------------|-------------------------------------|
| Process's PFF > upper bound                      | **Allocate more frames** to it      |
| Process's PFF < lower bound                      | **Reclaim frames** from it          |
| PFF > upper bound but no free frames in system   | **Suspend the process**             |

Compared to the working-set model:
- **Simpler to implement** — just count faults.
- **Directly controls** what we ultimately care about (the fault rate).
- Less informed about *why* the rate is what it is.

---

<br>

## 6. Memory Compression

An alternative to writing dirty pages out to disk: **compress them in RAM**.

- Collect several pages, run them through a compressor (Apple WKdm, Microsoft Xpress), and **pack the compressed forms into a single frame**.
- Trade some CPU for a *lot* less disk I/O. On modern hardware, compression is often faster than SSD paging — and on mobile devices with limited or no swap, it is essentially the only option.

```
Free-frame list                  After compression:
[F12][F45][F88][F03]             [Compressed page-pack F99]
                                 [F12][F45][F88] released back to free-frame list
```

When a compressed page is referenced again, the OS decompresses it back into a fresh frame. This costs a page fault and a decompress step, but no disk I/O.

Used by **macOS, iOS, Android, Windows 10+**. Compression ratios typically 30–50% — a 4 KB page becomes 1.2–2 KB.

---

<br>

## 7. Kernel Memory Allocation

### 7.1 Why Kernel Memory Is Special

The kernel can't use the same paging-based allocator it gives to user processes. Two reasons:

1. **Many tiny objects.** Kernel structures (PCBs, file objects, semaphores, network packets) are often *smaller than a page*. Allocating a whole frame per object would waste 90%+ of memory.
2. **Some hardware demands physically contiguous memory** — DMA controllers in particular. Page-level allocation gives you pages, but not necessarily *contiguous* pages.

Two classical strategies coexist in real kernels:

- **Buddy system** — coarse-grained, contiguous, fast coalesce.
- **Slab allocation** — fine-grained, per-object-type caches.

### 7.2 Buddy System

Memory is managed in **power-of-two-sized blocks** within a contiguous physical region.

- A request of size $r$ is rounded up to the next power of 2.
  - Example: 21 KB request → 32 KB block.
- If no block of that size exists, recursively split a larger one in half (each half being a "buddy" of the other) until you get the right size.

```
Allocating 21 KB from a 256 KB segment:

   [           256 KB             ]
   [   128 KB AL  ][   128 KB AR  ]   ← split
   [ 64 BL ][ 64 BR ][   128 KB   ]   ← split AL
   [32 CL][32 CR][ 64 BR ]...         ← split BL — 32 KB allocated for 21 KB request
```

**Deallocation triggers coalescing:** when a block is freed and its buddy is also free, they merge back into the larger size — recursively up the tree.

- **Pro:** coalescing is trivial and fast (constant-time per level, $O(\log n)$ total).
- **Con:** **internal fragmentation** — a 33 KB request still costs a 64 KB block (~50% waste).

### 7.3 Slab Allocation

**Per-object-type caches** keep pre-allocated, ready-to-use instances of a particular kernel data structure.

- **Slab** = one or more contiguous physical pages.
- **Cache** = a collection of slabs that all hold instances of a specific object type (e.g., "PCB cache", "file-object cache", "semaphore cache").
- Each cache has its own slabs and its own free list of objects.

```
Cache: "PCB"   ─── slab 1: [PCB][PCB][PCB][PCB]
                ── slab 2: [PCB][PCB][PCB][PCB]
                ── ...
```

**Operations:**
- Allocate → grab the first free object from the cache.
- Free → return the object to the cache's free list (no destruction, no re-init).

**Slab states:**
- **Full** — every object in the slab is in use.
- **Partial** — mixed.
- **Empty** — every object free.

Allocator preference: **partial → empty → allocate a new slab**.

**Advantages:**
- **Zero internal fragmentation** — slot size exactly matches object size.
- **Fast** — no malloc/free cost; reuses pre-initialized objects.
- **Cache-friendly** — same-type objects clustered together.

Linux variants:
- **SLAB** — the original Linux implementation.
- **SLOB** — minimal/list-based, for memory-constrained embedded systems.
- **SLUB** — default since Linux 2.6.24; less metadata overhead, better SMP scaling.

---

<br>

## 8. Other Considerations

### 8.1 Prepaging

Pure demand paging suffers a flood of faults at process startup. **Prepaging** preloads pages that are likely to be needed soon:

- On process suspension, remember the working set.
- On resume, load all those pages **before** the first instruction executes.

**Cost-benefit:** prepage $s$ pages, of which a fraction $\alpha$ end up actually used.
- Useful work saved: $s \cdot \alpha$ faults avoided.
- Wasted work: $s \cdot (1 - \alpha)$ pages loaded that are never used.
- Beneficial when $\alpha$ is close to 1.

Linux's **`readahead()`** is a related mechanism — explicitly tells the kernel "I'm about to read this range of a file; please pre-fault it."

### 8.2 Program Structure and Locality

Programmer-controlled locality can dwarf any OS optimization. Consider a 128 × 128 `int` array where one row exactly fills one page:

```c
// BAD — column-major access on a row-major language: jumps to a new page on every access
int data[128][128];
for (int j = 0; j < 128; j++)
    for (int i = 0; i < 128; i++)
        data[i][j] = 0;          // 128 × 128 = 16,384 page faults!
```

```c
// GOOD — row-major access matches storage layout
int data[128][128];
for (int i = 0; i < 128; i++)
    for (int j = 0; j < 128; j++)
        data[i][j] = 0;          // only 128 page faults
```

Data structure locality matters too:
- **Stack** — excellent locality (always touches the top).
- **Hash table** — poor locality (references scatter by design).

### 8.3 I/O Interlock and Page Locking

A subtle problem with global replacement and concurrent I/O:

```
Process A: starts a disk READ into buffer page X of process A
Process B: page-faults; OS picks page X as the victim (global replacement!)
         OS rewrites page X with process B's new contents
DMA completes: writes the original disk read into the (now hijacked) page X
              → corruption!
```

**Solutions:**

- **Indirect I/O via kernel buffers** — always read into kernel memory first, then copy to user. Safe but adds a copy.
- **Lock bit (pin)** — set a "this frame is locked, do not replace" bit. The page-replacement algorithm skips locked frames. Unlock when the I/O completes. Kernel pages are typically permanently pinned for the same reason.

---

<br>

## 9. Lab — Page-Replacement Algorithm Simulator

**Goal:** simulate **FIFO, LRU, OPT** on the same reference string and the same number of frames; compare page-fault counts.

**Requirements:**
- **Input:** a reference string (integer array) and a frame count.
- **Output for each algorithm:** the frame state after each reference, with `* Fault` marked when applicable.
- **Final output:** a comparison table.

**Input example:**
```
Reference String: 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
Number of Frames: 3
```

**Core FIFO sketch in Python:**

```python
def fifo(ref_string, num_frames):
    frames = []
    faults = 0
    for page in ref_string:
        if page not in frames:
            faults += 1
            if len(frames) < num_frames:
                frames.append(page)
            else:
                frames.pop(0)        # evict oldest
                frames.append(page)
        print(f"Ref: {page}  Frames: {frames}  {'* Fault' if page not in frames else ''}")
    return faults
```

**Expected output:**

```
=== FIFO Algorithm ===
Ref: 7  Frames: [7, -, -]  * Page Fault
Ref: 0  Frames: [7, 0, -]  * Page Fault
Ref: 1  Frames: [7, 0, 1]  * Page Fault
Ref: 2  Frames: [2, 0, 1]  * Page Fault
Ref: 0  Frames: [2, 0, 1]
...
Total Page Faults: 15

=== Comparison ===
| Algorithm | Page Faults |
|-----------|-------------|
| FIFO      | 15          |
| LRU       | 12          |
| Optimal   |  9          |
```

---

<br>

## Summary

| Topic | Key Content |
|-------|-------------|
| **Virtual memory** | Decouples logical from physical address space; programs may exceed RAM |
| **Demand paging** | Pager (lazy) loads pages only on reference; valid/invalid bit + page-fault trap |
| **EAT** | $(1-p)\cdot \text{mem} + p\cdot \text{pf}$; $p$ must be tiny ($\lesssim 10^{-5}$) |
| **Copy-on-Write** | `fork()` shares pages, copies only on write; `vfork()` skips copying entirely |
| **Page replacement** | FIFO, OPT, LRU; dirty bit halves write-backs |
| **Belady's anomaly** | FIFO can fault more with *more* frames; stack algorithms (LRU/OPT) cannot |
| **LRU approximation** | Reference bit + clock; enhanced clock uses (R, M) classes |
| **Frame allocation** | Equal vs proportional; minimum bound from instruction architecture |
| **Replacement scope** | Global (high throughput, unpredictable) vs Local (predictable, may underuse memory) |
| **Thrashing** | Too few frames → fault explosion → CPU utilization collapse |
| **Working-set / PFF** | Track recent references / fault rate to detect and prevent thrashing |
| **Memory compression** | Compress pages in RAM instead of swapping to disk |
| **Kernel memory** | Buddy system (powers of 2, contiguous, easy coalesce) + slab (per-object cache) |
| **Other** | Prepaging, program-structure locality, page pinning for I/O |

**Key takeaways:**
- **EAT is brutally sensitive to fault rate.** Every later technique exists to keep $p$ near zero.
- **Locality is the engine** that makes virtual memory work — without it, every algorithm here degrades to "random replacement."
- **OPT is the ceiling, LRU is the practical ideal, clock is what real systems use.** The hierarchy of approximations is a direct response to hardware constraints.
- **Belady's anomaly is FIFO's fault, not virtual memory's.** Stack algorithms eliminate it by construction.
- **Thrashing is the failure mode** of virtual memory — and the working-set / PFF models exist specifically to prevent it.
- **Kernel memory is its own world** because the kernel allocates lots of tiny structures and sometimes needs physical contiguity — buddy + slab is the textbook answer.

> "Virtual memory is the lie that makes every other lie in the OS possible. Page replacement is what keeps the lie believable."

**Next week:** Mass-Storage Structure — disk scheduling (FCFS, SCAN, C-SCAN, LOOK), RAID, storage device management.

---

<br>

## Self-Check Questions

1. **Definition:** What does it mean to "separate logical memory from physical memory," and what does that buy you?

   > **Answer:** Logical addresses live in each process's private virtual address space; physical addresses live in the shared DRAM. A page table maps the former to the latter, but **not every logical page needs to be mapped** at a given moment — absent pages are simply marked invalid in the PTE. This decoupling lets the OS (1) run programs larger than physical RAM by streaming pages in on demand, (2) keep many processes resident simultaneously, (3) start a process before it is fully loaded, (4) share physical frames between processes, and (5) implement fast `fork()` via copy-on-write. None of these are possible without the indirection.

2. **Page-fault walkthrough:** A process executes `MOV [0x4000], EAX`. The page containing 0x4000 is marked invalid. Walk through every step from CPU to instruction-restart.

   > **Answer:** (1) CPU forms the effective address 0x4000 and presents it to the MMU. (2) MMU walks the page table, finds the PTE has valid bit = 0, raises a **page-fault trap**. (3) Kernel trap handler examines the faulting address and PTE. (4) It distinguishes "illegal address" (deliver SIGSEGV) from "valid but on disk." Here, valid-but-on-disk. (5) Kernel pulls a free frame from the free-frame list (running the reaper if the list is empty). (6) Issues a disk I/O to read the page into that frame; process blocks waiting on I/O. (7) On I/O completion, kernel writes the new (frame #, valid=1, permissions) into the PTE. (8) Returns from trap, **restarting** the faulting `MOV` instruction. (9) MMU translates 0x4000 again — now valid — and the store succeeds.

3. **EAT impact:** With mem = 100 ns and pf = 5 ms, what page-fault rate keeps slowdown within 5%?

   > **Answer:** EAT = $(1-p)\cdot 100 + p \cdot 5{,}000{,}000$ = $100 + 4{,}999{,}900 p$. Slowdown within 5% means EAT ≤ 105, so $5 \geq 4{,}999{,}900 p$, giving $p \leq 10^{-6}$ — about **one fault per million memory accesses**. A spinning-disk page fault is so expensive that the fault rate must be in the parts-per-million range; SSDs relax this somewhat (~10⁻⁵ for the same 5% target with pf ~50 μs).

4. **Copy-on-write trace:** Parent has pages P1 (code, read-only) and P2 (data, writable). Parent calls `fork()`; child writes to P2. Describe the frame and PTE state at each step.

   > **Answer:** **Before fork:** parent's PTEs point to frames F1 (P1, RO) and F2 (P2, RW). **fork():** kernel copies parent's PTEs into child's page table. Both processes now have PTEs pointing at F1 (still RO — code, never needed COW) and F2 (which the kernel **marks RO in both PTEs**, even though P2 is logically writable). **Child writes to P2:** MMU sees RO PTE, raises **protection fault**. Kernel allocates a new frame F2'; copies F2 → F2'; updates child's PTE for P2 to point at F2' and mark it RW. Parent's PTE for P2 is also flipped back to RW (so it doesn't fault on its next write). **Result:** F1 still shared (one frame for code); F2 and F2' now hold independent copies of the writable data — but only because the child wrote. If the child had `exec()`-ed instead, F2 would never have been duplicated.

5. **FIFO vs LRU vs OPT:** Run all three on reference string `1 2 3 4 1 2 5 1 2 3 4 5` with 4 frames. Tabulate fault counts.

   > **Answer:** **FIFO (4 frames)** — per the §3.6 trace, **10 faults**. **LRU (4 frames)**: 1,2,3,4 (4F, fill). Then 1,2 hit; 5 fault (LRU=3, evict 3) — frames {1,2,4,5}. 1,2 hit; 3 fault (LRU=4, evict 4) — {1,2,3,5}. 4 fault (LRU=5, evict 5) — {1,2,3,4}. 5 fault (LRU=1, evict 1) — {2,3,4,5}. **8 faults**. **OPT (4 frames)**: 1,2,3,4 fault (4). 1,2 hit. 5 fault — replace 3 (not used until later position) or 4? Future after step 7 is `1 2 3 4 5`: 4 next-use is later than 3, evict 4 — {1,2,3,5}. 1,2 hit. 3 hit. 4 fault — evict 5 (5 only used once more, far) actually 5 is reused later but 1,2 are not... best is replace 5? Future after "3 4 5": evict 1 (last use was step 5, all of 1,2 won't reappear) — {2,3,4,5} after evicting 1. 5 hit. **Total OPT ≈ 6 faults**. Comparison: FIFO 10 > LRU 8 > OPT 6 — and notice FIFO suffers Belady's anomaly here (9 with 3 frames, 10 with 4), while LRU/OPT are monotone.

6. **Belady's anomaly:** Why does FIFO suffer it while LRU does not? Use the stack-algorithm property to justify.

   > **Answer:** A replacement algorithm is a **stack algorithm** if, for every prefix of any reference string, the resident set with $n$ frames is a subset of the resident set with $n+1$ frames. **LRU** trivially satisfies this — with $n$ frames the resident set is "the $n$ most-recently-referenced pages," which is a subset of "the $n+1$ most-recently-referenced pages." Adding a frame can only add to the resident set, never change which other pages are kept; therefore page faults can only stay the same or decrease. **FIFO** is *not* a stack algorithm: the resident set with $n$ frames depends on insertion order, and with $n+1$ frames the order shifts so that a page evicted in the $n$-frame run may *not* be in the $n+1$-frame resident set. Different eviction patterns → Belady's anomaly is possible.

7. **Reference bit + clock:** 5 pages in a circular queue with reference bits (1,1,0,1,0). The hand points at page 1. We need to evict. Walk the clock pointer; which page is evicted?

   > **Answer:** Start at page 1, bit=1 → clear to 0, advance. Page 2, bit=1 → clear to 0, advance. Page 3, bit=0 → **VICTIM**: evict page 3. (Pages 1 and 2 survive having received "second chances," and their bits are now 0 — they will be the next victims if no further references occur. Pages 4 and 5 are untouched this pass.) Final state of bits before insertion: (0, 0, ?, 1, 0); page 3 is replaced with the new page (typically with bit=0).

8. **Enhanced second-chance:** A frame is (R=0, M=1). What does this say about the page, and what is the eviction cost compared to (R=0, M=0)?

   > **Answer:** R=0 means the page has not been referenced in the current cycle ("not recently used"); M=1 means it has been written to since being loaded ("dirty"). So this page is **stale but modified**. Compared to (R=0, M=0) — also stale, but **clean** — evicting (0, 1) is more expensive because the dirty contents must be **written back to swap/disk** before the frame can be reused. The clean page can simply be dropped (the disk already has the same bytes). The enhanced-second-chance algorithm prefers to evict (0, 0) first precisely to avoid the write-back, eviting (0, 1) only when no clean stale page is available.

9. **Working-set:** $\Delta = 8$, reference string `1 2 1 3 4 5 1 2 3 4 5 6`. What is the working set after the last reference?

   > **Answer:** The last 8 references are `5 1 2 3 4 5 6` ... wait, let me count: positions 5–12 are `4, 5, 1, 2, 3, 4, 5, 6`. So $WS = \{1, 2, 3, 4, 5, 6\}$ — six distinct pages in the last 8 references. The process needs at least 6 frames to avoid thrashing in its current locality.

10. **Thrashing diagnosis:** A server's CPU utilization mysteriously drops from 80% to 15% after the OS admits one more process. Page-fault rate has spiked. What is happening, and what is the fix under (a) global replacement, (b) local replacement, (c) PFF?

    > **Answer:** Classic thrashing onset: total working-set demand $D = \sum WSS_i$ now exceeds physical frames $m$, so every process is missing pages, page-fault I/O dominates, and CPU idles waiting on disk. **(a) Global replacement:** the OS should *suspend* one process (typically the most-recently-admitted, or the one with the largest fault rate) until $D \leq m$; suspending frees the suspended process's frames for the others. **(b) Local replacement:** local already prevents one bad process from poisoning others — but the admission control failed. Fix: don't admit the new process if $\sum WSS_i + WSS_\text{new} > m$. **(c) PFF:** the page-fault rate of one or more processes will exceed the upper bound; the algorithm will try to give them more frames, fail (no free frames), and then suspend a process. Same outcome — the *signal* is just measured directly rather than inferred from working-set sizes.

11. **Buddy vs slab:** Why does the kernel need *both* allocators? What is each good and bad at?

    > **Answer:** **Buddy** handles **coarse-grained, contiguous physical memory** — pages and multi-page blocks. It coalesces freed buddies in $O(\log n)$, so fragmentation of large blocks recovers cleanly. But it suffers **internal fragmentation** for sub-page objects (a 17-byte allocation costs a 32-byte block, or worse, the page-sized minimum), and that is unacceptable for the *millions* of tiny kernel structures (PCBs, file objects, inodes). **Slab** complements buddy by sitting *on top of* it: a slab is one or more contiguous pages (obtained from buddy) divided into fixed-size object slots. Each slab cache is dedicated to one type, so the slot size exactly matches the object size — **zero internal fragmentation** for those objects, and allocation/free is O(1) free-list pop/push. Slab is bad at variable-sized large allocations (it would need a separate cache per size), which buddy handles naturally. So the kernel uses **buddy** for raw page-frame allocation and large contiguous blocks (e.g., DMA buffers), and **slab** for high-frequency typed objects.

12. **Program structure:** You have a 4 KB-page system and a `double M[1024][1024]` (each row = 8 KB = 2 pages). Compare the page-fault behavior of `for i, for j: M[i][j] = 0` versus `for j, for i: M[i][j] = 0`.

    > **Answer:** **Row-major access (`for i, for j`):** writes M[i][0], M[i][1], …, M[i][1023] in order. Each row is 2 pages; after 4 KB worth of writes (512 doubles), it walks onto the next page. Across the whole matrix: $1024 \text{ rows} \times 2 \text{ pages} = 2048$ pages — **2048 page faults** assuming no caching effects. **Column-major access (`for j, for i`):** writes M[0][j], M[1][j], …, M[1023][j] — each access is in a different row, so a different page (or pages). Per outer iteration of $j$, we touch 2048 pages (one per row, possibly two if $j$ straddles a page boundary). If we don't have 2048+ frames, every access faults: **up to 1024 × 1024 = ~10⁶ faults**. Speedup factor: **~500× more faults** for column-major. This is exactly why row-major access in C is non-negotiable for large arrays — the OS cannot recover from a row vs. column mismatch.
