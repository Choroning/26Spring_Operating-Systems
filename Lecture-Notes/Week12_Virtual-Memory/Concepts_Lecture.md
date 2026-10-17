# Week 12 Lecture — Virtual Memory

[Repository Guide](../../README.md) | [English](Concepts_Lecture.md) | [한국어](Concepts_Lecture.ko.md)

> **Last Updated:** 2026-10-18
>
> Operating System Concepts, Silberschatz, Ch 10

> **Learning Objectives:**
> 1. Define virtual memory and explain why separating the logical address space from the physical address space is useful
> 2. Explain demand paging, the role of the lazy swapper (pager), the page-fault handling sequence, pure demand paging, and the hardware required for instruction restart
> 3. Compute the Effective Access Time (EAT) under demand paging to quantify the cost of a single page fault, and describe copy-on-write (COW) and the difference between `fork()` and `vfork()`
> 4. Distinguish frame allocation from page replacement, apply FIFO, OPT, LRU, and the LRU-approximation algorithms (additional reference bits, second-chance (clock), enhanced second-chance, LFU, MFU) to a reference string, and explain how Belady's anomaly relates to stack algorithms
> 5. Compare equal and proportional allocation and global and local replacement, and define thrashing, the locality model, the working-set model, and page-fault frequency (PFF)
> 6. Explain memory compression, the buddy system, slab allocation for kernel memory, prepaging, the impact of program structure on locality, and I/O interlock with page locking

---

## Table of Contents

- [1. Background and Demand Paging](#1-background-and-demand-paging)
  - [1.1 What Virtual Memory Buys You](#11-what-virtual-memory-buys-you)
  - [1.2 The Virtual Address Space of a Process](#12-the-virtual-address-space-of-a-process)
  - [1.3 Shared Libraries via Virtual Memory](#13-shared-libraries-via-virtual-memory)
  - [1.4 Demand Paging and the Lazy Pager](#14-demand-paging-and-the-lazy-pager)
  - [1.5 Valid / Invalid Bit](#15-valid--invalid-bit)
  - [1.6 Page-Fault Handling Step by Step](#16-page-fault-handling-step-by-step)
  - [1.7 Pure Demand Paging and Hardware Requirements](#17-pure-demand-paging-and-hardware-requirements)
  - [1.8 The Difficulty of Instruction Restart](#18-the-difficulty-of-instruction-restart)
  - [1.9 Free-Frame List and Zero-Fill-on-Demand](#19-free-frame-list-and-zero-fill-on-demand)
  - [1.10 Demand-Paging Performance and EAT](#110-demand-paging-performance-and-eat)
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
  - [4.4 Clock Algorithm Visualization](#44-clock-algorithm-visualization)
  - [4.5 Enhanced Second-Chance and (R, M) Classes](#45-enhanced-second-chance-and-r-m-classes)
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
- [9. Lab: Page-Replacement Algorithm Simulator](#9-lab-page-replacement-algorithm-simulator)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. Background and Demand Paging

### 1.1 What Virtual Memory Buys You

**Virtual memory** is a technique that **separates logical (virtual) memory from physical memory**. It is built on top of paging (Week 11) and turns the page-table indirection into a powerful illusion: the process believes it has a huge, contiguous address space, while the OS quietly serves only the parts that are actually being used.

Virtual memory is worth the effort because of three structural wins:

- **Programs can be larger than physical RAM.** Only the pages actively touched need to be resident. Error-handling code, unused features, and over-sized declared arrays cost zero physical memory until they are referenced.
- **More processes fit in RAM simultaneously.** A higher degree of multiprogramming raises CPU utilization (Week 6).
- **Processes start faster.** A new process can start running as soon as a handful of pages are in memory; the rest are brought in on demand.

The same machinery also provides these features:

- Shared libraries are implemented with **shared frames** (the foundation is Week 11, Section 6.2).
- **Inter-process communication** works via shared memory.
- Copy-on-write implements a **fast `fork()`** (Section 2).

![Figure 1. Virtual memory larger than physical memory](../../images/Week12_p03a.png)

*Figure 1. Virtual memory larger than physical memory*

> **Note:** **Why this is the right abstraction.** The addressing layer of Week 11 (page tables, MMU, TLB) provides spatial virtualization. Demand paging adds temporal virtualization: a page only needs to exist in RAM at the moment the CPU references it, not before. Everything in this unit follows from the principle of making absent pages cheap.

### 1.2 The Virtual Address Space of a Process

Each process sees its own independent virtual address space. It typically starts from 0 and appears contiguous, but the underlying physical frames may be scattered anywhere in RAM, or partly on disk.

```text
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

*Figure 2. Layout of the virtual address space of one process*

- **Sparse address space**: large holes sit between text/data, heap, and stack.
- Holes occupy **no physical memory**, because they have no page-table entries pointing to frames.
- Shared libraries are usually mapped into the middle region between heap and stack.

![Figure 3. Virtual address space of text, data, heap, stack, and a hole](../../images/Week12_p03b.png)

*Figure 3. Virtual address space of text, data, heap, stack, and a hole*

> **Key Point:** Because holes cost nothing, processes can declare massive maximum sizes without paying for them. A 64-bit process can mmap a 2 TB sparse file, and only the touched pages are allocated frames.

### 1.3 Shared Libraries via Virtual Memory

Multiple processes can **share** libraries such as the standard C library:

- The library is mapped into each process's virtual address space at some virtual address. The address is often the same in every process for relocation simplicity, but it does not have to be.
- The **physical frames** that hold the library code are **shared**: one set of frames, many page-table entries pointing at them.
- Libraries are mapped **read-only**, which is what makes sharing safe (Week 11, Section 6.2).
- The same mechanism implements shared-memory IPC.
- `fork()` initially shares all pages between parent and child by copying the page table. Section 2 covers this as copy-on-write.

![Figure 4. Shared library using virtual memory](../../images/Week12_p04.png)

*Figure 4. Shared library using virtual memory*

### 1.4 Demand Paging and the Lazy Pager

**Demand paging** loads a page into memory **only when it is actually referenced**.

The OS component that performs the loading is called a **lazy swapper** or **pager**. As the name says, it is lazy: it never speculatively brings pages in.

Advantages:
- **Reduced I/O:** pages that are never touched are never read from disk.
- **Memory savings:** only the working set is resident.
- **Faster startup:** the process begins running as soon as the first instruction page is in.
- **More concurrent processes:** each one occupies less RAM.

### 1.5 Valid / Invalid Bit

Each page-table entry (PTE) carries a **valid/invalid bit**. It is the bit from Week 11, Section 6.1, now reinterpreted for demand paging:

- **Valid:** the page is legal and **currently resident in a physical frame**.
- **Invalid:** either the page is outside the process's address space, **or** it is a legal page that is currently on disk.

The CPU does not distinguish the two kinds of invalid page; both raise the same hardware trap. The OS decides which it is in the trap handler.

> **Note:** **Why this design is clean.** The check of whether a page exists and whether it is resident is fused with the protection check the MMU already performs. Once the MMU supports the valid bit, demand paging needs no new datapath, only new OS logic on the trap. The instruction-restart capability it also needs is covered in Section 1.7.

![Figure 5. Page table when some pages are not in main memory](../../images/Week12_p05.png)

*Figure 5. Page table when some pages are not in main memory*

### 1.6 Page-Fault Handling Step by Step

When the CPU touches a page whose PTE is invalid, a **page-fault trap** transfers control to the kernel:

```text
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

The whole sequence is **orders of magnitude slower** than a normal memory access (about 100 ns): typically 5-10 ms on a spinning disk, hundreds of μs on an SSD, and tens of μs on NVMe. This is why the page-fault rate must be kept tiny.

![Figure 6. Steps in handling a page fault](../../images/Week12_p06.png)

*Figure 6. Steps in handling a page fault*

### 1.7 Pure Demand Paging and Hardware Requirements

**Pure demand paging** is the extreme strategy of starting the process with **no** pages in memory. Even the first instruction triggers a page fault. As execution proceeds, **locality of reference** (defined precisely via the locality model in Section 5.7) rapidly populates the working set, and the fault rate plummets.

The **hardware** required to support demand paging:

- A **page table** with a valid/invalid bit (and protection bits).
- **Secondary memory (swap space)** to hold non-resident pages.
- **Instruction restart capability**. The CPU must be able to re-execute the trapped instruction cleanly after the fault is serviced.

### 1.8 The Difficulty of Instruction Restart

Restarting an instruction is conceptually simple but architecturally tricky:

- **Easy case:** if the fault occurs during instruction fetch, just re-fetch and start over.
- **Hard case:** consider the multi-step instruction `ADD A, B, C`:
  1. Fetch and decode the instruction.
  2. Fetch the operand at A.
  3. Fetch the operand at B.
  4. Compute A + B.
  5. Store the result to C. **The page fault happens here.**
  - Naive re-execution repeats steps 1-4, which is fine because they have no side effects. But what about instructions that **do** have side effects?
- **IBM MVC instruction:** copies up to 256 bytes from one location to another. If the source or destination block crosses a page boundary and the next page is invalid, a fault occurs mid-copy. If the two blocks overlap, the source may already have been modified, so the instruction cannot simply be restarted either.
  - **Fix 1:** before any byte is written, probe both end addresses; if either would fault, take the fault **before** any side effect.
  - **Fix 2:** save overwritten destination bytes into temporary registers; on a fault, restore them before transferring control to the trap handler.

> **Note:** **Historical significance.** Whether instructions can be restarted determines whether demand paging is possible on a given architecture at all. Some early CPUs could not support it cleanly, which is why virtual memory took a generation of hardware design to become universal.

### 1.9 Free-Frame List and Zero-Fill-on-Demand

The OS maintains a **free-frame list**, a pool of physical frames available for allocation.

- At system boot, **all** user-available memory is registered in the list.
- Page-fault handling pulls one frame off the list and uses it.
- When the list shrinks below a threshold, **page replacement** (Section 3) refills it by evicting victims.

**Zero-fill-on-demand**: before handing a frame to a new process, the kernel **zeroes it out**. The reason is security: the frame previously belonged to some other process, and its leftover contents must not leak to the new one.

![Figure 7. Structure of the free-frame list](../../images/Week12_p08.png)

*Figure 7. Structure of the free-frame list*

### 1.10 Demand-Paging Performance and EAT

Let:
- $p$: page-fault rate, $0 \leq p \leq 1$
- $\text{mem}$: memory access time
- $\text{pf}$: page-fault service time (interrupt handling, disk I/O, and process restart)

$$
\text{EAT} = (1 - p) \cdot \text{mem} + p \cdot \text{pf}
$$

**Worked example:** with $\text{mem} = 200\ \text{ns}$ and $\text{pf} = 8\ \text{ms} = 8\,000\,000\ \text{ns}$:

$$
\text{EAT} = 200 + 7\,999\,800 \cdot p
$$

- With $p = 1/1000$ (one fault per 1000 accesses), $\text{EAT} = 200 + 7999.8 \approx 8200\ \text{ns}$, which is **about 40× slower** than raw memory.
- To keep the slowdown within 10% ($\text{EAT} \leq 220$), we need $20 \geq 7\,999\,800 \cdot p$, that is, $p \leq 0.0000025 \approx 1/400\,000$.

> **Note:** **The lesson.** Page faults must be **extremely rare** (at most one per about 400,000 references) for demand paging to feel free. This is why good replacement policies, working sets, and locality management matter: a small jump in fault rate translates into a huge jump in effective access time.

> **[Computer Architecture]** This EAT formula is the same expected-value technique used for the **TLB** in Week 11 ($\alpha(T+M) + (1-\alpha)(T+2M)$), just one rung lower on the **memory hierarchy**. The TLB version balanced a cache hit (cycles) against a memory access (about 100 ns); this version balances a memory access (about 100 ns) against a fault serviced by disk or SSD (millions of cycles). The numbers are brutal here because each step down the hierarchy (register, cache, DRAM, SSD, disk) costs roughly **two orders of magnitude** more latency. A TLB miss merely doubles a fast operation, while a page fault replaces a 100 ns operation with an 8 ms one. That is why the tolerable miss rate collapses from about 2% (TLB) to about 0.00025% (page fault).

### 1.11 Swap Space Utilization

Swap space is a region on disk, and reads from it are typically faster than ordinary file-system reads because:
- The OS uses **larger block transfers** with no per-file lookup overhead.
- There is no file-system metadata indirection.

There are two strategies for where a page lives:

- **Strategy 1:** at process start, copy the whole executable into swap space, then demand-page from swap.
- **Strategy 2:** demand-page from the file system initially; when a page is evicted, write the modified (dirty) version to swap and read it back from swap thereafter. **Linux and Windows** use this approach.

Details:
- **Binary executable pages** (text, read-only data) come from the file system. Since they are never modified, on eviction they are simply discarded and re-read from the file system next time.
- **Anonymous memory** (memory with no file backing, such as stack and heap) must be written to swap on eviction. This is where swap space is actually used.

Mass-storage structure, including swap-space management, disk scheduling (FCFS, SCAN, C-SCAN, LOOK), and RAID, is covered in Week 13.

---

<br>

## 2. Copy-on-Write

### 2.1 The COW Idea

A naive `fork()` duplicates the parent's entire address space into a fresh set of frames for the child. This is wasteful, since the child very often calls `exec()` immediately and throws all of it away.

**Copy-on-write (COW)** defers the copy until it is actually necessary:

```text
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

**Linux, Windows, and macOS** use COW. The performance win is large for workloads that call `exec()` right after `fork()` (shells, servers).

> **Note:** Step 3 above is simplified. Depending on the implementation, the other process's PTE may stay read-only; when that process later tries to write, the fault handler sees that the frame is no longer shared and simply restores write permission without copying.

### 2.2 Non-Modifiable Pages and vfork()

- **Non-modifiable pages** (executable code) never need COW, because they are always shared and read-only by nature. COW is only relevant for pages that **might** be written.
- **`vfork()` (virtual memory fork)** is a more aggressive variant:
  - The parent process is **suspended** until the child terminates or calls `exec()`.
  - Pages are **shared without any copying or COW marking**. The child literally runs in the parent's address space.
  - It is extremely fast because not even the page table is copied, but it is **dangerous**: if the child does anything other than minimal setup followed by `exec()`, it corrupts the parent's address space.
  - It is used in classical UNIX shell implementations, where the child immediately replaces itself with another program.

---

<br>

## 3. Page Replacement

### 3.1 Why Replacement Is Needed

Demand paging leads to **over-allocation**. The OS keeps loading pages as faults occur, and eventually all frames are in use; a new page fault then has nowhere to put the incoming page.

**Page replacement** picks a resident page to evict (the **victim**) to make room for the incoming page.

Page replacement completes the illusion of virtual memory. Because the OS continuously rotates the 10 most useful pages in and out, a 20-page process can run with only 10 frames.

![Figure 8. Need for page replacement](../../images/Week12_p14.png)

*Figure 8. Need for page replacement*

### 3.2 Basic Replacement Procedure and the Dirty Bit

```text
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

> **Note:** **Why the dirty bit matters.** Writing back the victim (swap-out) is, together with the page-in, the most expensive disk I/O in page-fault handling. Replacing a modified page needs two disk transfers (write-back and page-in), whereas a clean page skips the write-back, which halves the I/O time of that fault.

![Figure 9. Page replacement as victim eviction and new-page loading](../../images/Week12_p15.png)

*Figure 9. Page replacement as victim eviction and new-page loading*

### 3.3 What Page Replacement Buys You

Page replacement is what **truly decouples logical memory from physical memory**. Without it, a process could only run if its entire address space fit in RAM. With it:

- A 20-page process runs on 10 frames.
- Many processes coexist comfortably even when their virtual sizes exceed RAM in total.

Two algorithmic problems must be answered:

1. **Frame allocation:** decide how many frames each process gets (Section 5).
2. **Page replacement:** given a fixed frame budget, decide which page to evict (Sections 3.5-4).

The goal is always to **minimize the page-fault rate** (recall the EAT calculation in Section 1.10).

### 3.4 Reference Strings

To compare algorithms fairly, we evaluate them against a **reference string**, the sequence of page numbers a process touches.

**Example:** with a page size of 100 bytes, consider the byte addresses
```text
0100, 0432, 0101, 0612, 0102, 0103, ...
```
Converting addresses to page numbers, and merging consecutive references to the same page (the later ones cannot fault), collapses them to the reference string
```text
1, 4, 1, 6, 1, 6, 1, 6, 1, 6, 1
```

We then simulate each algorithm against the same string and count page faults. Generally, **more frames mean fewer faults** (with the famous exception in Section 3.6).

The canonical example reference string used below:
```text
7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1
```

### 3.5 FIFO

**FIFO** (First-In, First-Out) replaces the page that has been in memory the longest.

- It is managed with a FIFO queue: new pages enter the tail, and victims leave the head.
- It is trivial to implement, but treats all pages identically regardless of recent activity.

**FIFO trace:** 3 frames, reference string `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1`

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

*Table 1. FIFO trace of the canonical reference string with 3 frames*

**Total: 15 page faults.**

### 3.6 Belady's Anomaly

One would expect that **adding frames always reduces faults**. With FIFO, this is **not always true**.

**Reference string:** `1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5`

| Frames | Page Faults |
|--------|-------------|
| 3      | **9**       |
| 4      | **10**      |

*Table 2. FIFO page-fault counts by number of frames*

More frames lead to **more** faults. This is **Belady's anomaly**, a fundamental pathology of FIFO.

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

*Table 3. FIFO trace of the Belady example with 3 frames*

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

*Table 4. FIFO trace of the Belady example with 4 frames*

![Figure 10. Page-fault curve for FIFO replacement and Belady's anomaly](../../images/Week12_p18.png)

*Figure 10. Page-fault curve for FIFO replacement and Belady's anomaly*

### 3.7 Optimal (OPT / MIN)

The **OPT** algorithm replaces the page that **will not be used for the longest time in the future**.

- It provably gives the **lowest possible page-fault rate**; no algorithm can do better.
- **No Belady anomaly:** adding frames never increases faults.
- It is **impossible to implement** in practice, because it requires perfect knowledge of the future.
- It is used as a **baseline** (upper bound on quality) for evaluating real algorithms.

**OPT trace:** 3 frames, same string as FIFO

| Ref | F1    | F2    | F3    | Fault? | Reason (victim = farthest next use) |
|-----|-------|-------|-------|--------|-------------------------------------|
| 7   | **7** | -     | -     | F      |        |
| 0   | 7     | **0** | -     | F      |        |
| 1   | 7     | 0     | **1** | F      |        |
| 2   | **2** | 0     | 1     | F      | next use 7→#18, 0→#5, 1→#14, so evict 7 |
| 0   | 2     | 0     | 1     |        |        |
| 3   | 2     | 0     | **3** | F      | next use 2→#9, 0→#7, 1→#14, so evict 1 |
| 0   | 2     | 0     | 3     |        |        |
| 4   | 2     | **4** | 3     | F      | next use 2→#9, 0→#11, 3→#10, so evict 0 |
| 2   | 2     | 4     | 3     |        |        |
| 3   | 2     | 4     | 3     |        |        |
| 0   | 2     | **0** | 3     | F      | next use 2→#13, 4→never, 3→#12, so evict 4 |
| 3   | 2     | 0     | 3     |        |        |
| 2   | 2     | 0     | 3     |        |        |
| 1   | 2     | 0     | **1** | F      | next use 2→#15, 0→#16, 3→never, so evict 3 |
| 2   | 2     | 0     | 1     |        |        |
| 0   | 2     | 0     | 1     |        |        |
| 1   | 2     | 0     | 1     |        |        |
| 7   | **7** | 0     | 1     | F      | next use 2→never, 0→#19, 1→#20, so evict 2 |
| 0   | 7     | 0     | 1     |        |        |
| 1   | 7     | 0     | 1     |        |        |

*Table 5. OPT trace of the canonical reference string with 3 frames*

**Total: 9 page faults**, six fewer than FIFO. At every fault the victim is the resident page whose **next use lies farthest in the future** (a page never used again counts as infinitely far). Reference positions are 1-indexed against the string `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1`.

### 3.8 LRU (Least Recently Used)

Since we cannot see the future, we approximate it with the past: **replace the page that has not been used for the longest time**.

- **Key assumption:** locality of reference. A page that has been idle for a long time is unlikely to be needed soon.
- It picks the farthest page looking backward instead of forward, so it is OPT run backward in time.
- **No Belady anomaly:** LRU is a **stack algorithm** (Section 3.10).

**LRU trace:** 3 frames, same reference string

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
| 0   | **0** | 3     | 2     | F      | 4 least recent      |
| 3   | 0     | 3     | 2     |        |                     |
| 2   | 0     | 3     | 2     |        |                     |
| 1   | **1** | 3     | 2     | F      | 0 least recent      |
| 2   | 1     | 3     | 2     |        |                     |
| 0   | 1     | **0** | 2     | F      | 3 least recent      |
| 1   | 1     | 0     | 2     |        |                     |
| 7   | 1     | 0     | **7** | F      | 2 least recent      |
| 0   | 1     | 0     | 7     |        |                     |
| 1   | 1     | 0     | 7     |        |                     |

*Table 6. LRU trace of the canonical reference string with 3 frames*

**Total: 12 page faults.** In comparison, **FIFO 15 > LRU 12 > OPT 9**.

### 3.9 LRU Implementation

True LRU needs to know the **relative recency** of every resident page. There are two hardware-supported implementations.

**Method 1: Counter (time stamp).**
- Each PTE stores a time-of-use field.
- A logical clock is added to the CPU; it ticks on every memory access.
- On every reference, the clock value is copied into the referenced page's PTE.
- The victim is the page with the smallest time stamp.
- **Cost:** a memory write (the PTE) on every access, and a linear scan of resident pages to find the minimum.

**Method 2: Stack (doubly linked list).**
- A doubly linked list of resident pages is kept in recency order.
- On a reference, the page moves to the **top** of the stack; this changes six pointers in the worst case.
- The victim is the page at the **bottom** of the stack, found in O(1) without a scan.

Both demand hardware help. Doing this in software via interrupts would slow every memory access by a factor of at least ten.

> **[Data Structures]** Method 2 is exactly a **doubly linked list** used as a recency-ordered queue, and the figure of six pointer updates is the standard cost of an unlink-then-push-to-front: detaching a node touches its predecessor's and successor's pointers, and reinserting at the head touches the old head, the new node's two links, and the head pointer. A doubly (not singly) linked list is required because LRU must **remove a node from the middle in O(1)**, and an interior node can only be spliced out in constant time if it carries a back-pointer to its predecessor. Pairing this list with a hash map from page number to node is precisely the textbook **LRU cache** design (O(1) lookup, O(1) move-to-front, O(1) evict-from-tail).

![Figure 11. Stack recording the most recent page references](../../images/Week12_p21.png)

*Figure 11. Stack recording the most recent page references*

### 3.10 Stack Algorithms and Why They Avoid Belady

A replacement algorithm is a **stack algorithm** if, at every point of every reference string, the set of resident pages with $n$ frames is a **subset** of the set with $n + 1$ frames.

- **LRU is a stack algorithm.** With $n$ frames, the resident set is exactly the $n$ most recently referenced pages. With $n + 1$ frames, those $n$ pages are still in the resident set, plus one more.
- **OPT is a stack algorithm** as well (by a symmetric argument on the future).
- **FIFO is not a stack algorithm.** The resident set with $n$ frames is not necessarily a subset of the one with $n + 1$ frames, and this is exactly what allows Belady's anomaly.

**Theorem (intuition):** for a stack algorithm, adding a frame can **never increase** the number of faults. A reference that hits with $n$ frames must also hit with $n + 1$ frames, because the resident set at that moment is contained in the larger resident set. Hence the faults of the larger memory are a subset of the faults of the smaller one.

### 3.11 Algorithm Comparison

Same reference string, 3 frames:

| Algorithm | Page Faults | Notes |
|-----------|-------------|-------|
| FIFO      | **15**      | Simple but suffers Belady's anomaly |
| LRU       | **12**      | Good performance, expensive implementation |
| OPT       | **9**       | Optimal but unimplementable |

*Table 7. Page-fault counts of FIFO, LRU, and OPT with 3 frames*

---

<br>

## 4. LRU-Approximation Algorithms

### 4.1 Why Approximate?

True LRU requires hardware that updates a clock or stack on **every** memory reference, and most architectures do not provide that. What most architectures **do** provide is a **reference bit**:

- It is set to 1 by hardware whenever the page is read or written.
- It is cleared by the OS on its own schedule.
- It tells us **which pages were used** in some recent window, but not the **order**.

LRU-approximation algorithms use this single bit (or a few bits) to make replacement decisions that approximate LRU at a fraction of the cost.

> **[Computer Architecture]** The **reference bit** and **dirty (modify) bit** are stored in the **page-table entry** that the MMU already reads during each translation (Week 11). Hardware sets them as a side effect of memory access, so they add no access-time cost. Exact LRU would require an extra write on every memory reference to update a timestamp or reorder a list, which the datapath cannot handle. Real systems therefore approximate LRU, and the clock algorithm extracts as much information as possible from the single reference bit that hardware sets for free.

### 4.2 Additional-Reference-Bits

The OS maintains an **8-bit byte** (shift register) per page and updates it on a periodic timer interrupt (say, every 100 ms):

```text
On each timer tick:
  1. Shift the current reference bit into the MSB of the page's byte.
  2. Shift the rest of the byte right by one.
  3. Clear the hardware reference bit (ready for the next interval).
```

Read the byte left to right: the **MSB (bit 7) is the most recent interval**, and each step to the right is one interval further into the past (bit 0 is the oldest tracked interval). A `1` in a position means the page was referenced during that interval.

```text
Bit position:  7 6 5 4 3 2 1 0   ← MSB = most recent interval, LSB = oldest
Example histories:
  1 1 0 0 0 1 0 0   →  used in the last 2 intervals, idle for 3, then used once long ago
  0 1 1 1 0 1 1 1   →  NOT used in the most-recent interval, but heavier use further back

Compared as unsigned integers, 11000100 (196) > 01110111 (119)
→ the second page is the better eviction candidate (smaller value = less recent use)
```

Because the most recent interval sits in the highest-value bit, ordinary unsigned comparison automatically ranks recent use above use long ago: the larger the integer, the more recent the activity.

- The victim is the page with the **smallest** byte value, that is, the least recently used one.
- Ties are broken by FIFO order.
- Increasing the number of history bits improves accuracy but raises the interrupt cost.

### 4.3 Second-Chance (Clock) Algorithm

Combine **FIFO** with the **reference bit**:

```text
Maintain pages in a FIFO order (a circular queue).
On replacement need:
   Look at the head page of the FIFO.
   If reference bit = 0 → REPLACE this page.
   If reference bit = 1 → clear the bit, treat the page as "newly arrived,"
                          advance to the next page in the FIFO, and repeat.
```

It is implemented as a **circular queue** with a hand pointer. The algorithm looks exactly like a clock with a sweeping hand, hence the name **Clock Algorithm**.

> **[Data Structures]** The hand is just an index or pointer into a **circular buffer** (or a circular singly linked list whose tail links back to the head). Advancing the hand is the classic `i = (i + 1) % n` wrap-around. There is no front or back, so unlike the plain FIFO queue of Section 3.5, nothing is ever shifted or re-enqueued; clearing a reference bit is an O(1) in-place mutation, and the hand simply moves on. This structure makes giving a page a second chance cost nothing: a real FIFO would have to dequeue and re-append the survivor, whereas the circular list leaves it in place and only moves the cursor.

- Worst case (all bits = 1): one full revolution clears every bit, and the next pass behaves like plain FIFO.
- Frequently used pages have reference bit = 1 whenever the hand reaches them, so they are rarely replaced.

> **Note:** **Why clock works.** The reference bit gives a binary signal of whether a page was recently used. The clock turns that single bit into something close to LRU: recently used pages get a free pass, and only pages that have gone unused for a long time are evicted.

![Figure 12. Second-chance (clock) page-replacement algorithm](../../images/Week12_p23.png)

*Figure 12. Second-chance (clock) page-replacement algorithm*

### 4.4 Clock Algorithm Visualization

```text
[Initial state]                [After search]
   v (pointer)                    v (pointer, points to new victim)
+------+                    +------+
| P1: 1| → clear to bit=0   | P1: 0|
| P2: 1| → clear to bit=0   | P2: 0|
| P3: 0| → VICTIM!          | NEW  | ← incoming page inserted here
| P4: 1|                    | P4: 1|
| P5: 0|                    | P5: 0|
+------+                    +------+

P1, P2 survive — given a "second chance" by clearing their bit.
P3 is replaced because its bit was already 0.
```

*Figure 13. Clock search that clears the bits of P1 and P2 and replaces P3*

### 4.5 Enhanced Second-Chance and (R, M) Classes

The clock is refined by using both the **reference bit (R)** and the **modify (dirty) bit (M)**. Each page falls into one of **4 classes**:

| Class | (R, M) | Meaning                                | Replacement priority |
|-------|--------|----------------------------------------|----------------------|
| 1     | (0, 0) | Not recently used, not modified        | **Highest** (cheap to drop) |
| 2     | (0, 1) | Not recently used, but modified        | 2nd (needs write-back)  |
| 3     | (1, 0) | Recently used, not modified            | 3rd                    |
| 4     | (1, 1) | Recently used **and** modified         | **Lowest** priority    |

*Table 8. (R, M) classes and replacement priority of enhanced second-chance*

Pages in lower-numbered classes are evicted first. Replacing a modified page costs an additional disk write, so clean pages are evicted first, and pages that are both recent and dirty are evicted last.

### 4.6 LFU and MFU

Count how many times each page has been referenced:

- **LFU (Least Frequently Used):** evict the page with the **fewest** references.
  - Problem: a page used heavily during initialization and then never again hangs around because its count is huge.
  - Fix: periodically right-shift every counter (exponential decay), so old activity fades.
- **MFU (Most Frequently Used):** evict the page with the **most** references.
  - Logic: pages with small counts just arrived and probably will be used; the heavily referenced ones have already done their work.

Neither approximates OPT well in general. They are **rarely used in practice**, because the implementation cost is high relative to the modest benefit.

### 4.7 Page Buffering

Several complementary techniques can be layered on top of any replacement policy:

- **Maintain a free-frame pool.** On a page fault, take a frame from the pool **immediately** instead of waiting for the replacement algorithm. The victim is written back **later, asynchronously**. The faulting process restarts quickly.
- **Modified-page list (batched write-back).** Collect dirty pages and write them out in a batch when the paging device is idle.
- **Cache victims in the free-frame pool.** When a frame is freed, remember which page it used to hold. If that page is needed again **before** the frame is reused, recover it without any disk I/O. Some UNIX-style systems use this together with second-chance.

---

<br>

## 5. Frame Allocation and Thrashing

### 5.1 Minimum Frames and Where the Bound Comes From

Each process needs at least a **minimum number of frames** so that any **single** instruction, together with every page its operands touch, can run to completion after a page-fault restart. If even one instruction needed more distinct pages than the process owns, every restart would fault again on the same instruction, and the process could never make progress.

- The minimum is **determined by the architecture**: it is the maximum number of distinct pages any single instruction can reference (the instruction's own page plus all of its operand pages).
- Indirect addressing increases the bound. For example, on a machine whose memory-reference instructions reference only one memory address, allowing one level of indirect addressing requires at least **3 frames** (instruction page, operand-pointer page, operand page).

The **maximum** is bounded by the available physical memory.

### 5.2 Equal vs Proportional Allocation

Given $m$ available frames and $n$ processes, the OS must decide how to distribute them.

- **Equal allocation:** $m / n$ frames each.
  - Example: 93 frames and 5 processes give 18 frames each, with 3 spare (the spare frames can serve as a free-frame buffer pool).
- **Proportional allocation:** give a process frames in proportion to its size $s_i$:
  $$ a_i = \frac{s_i}{S} \cdot m, \qquad S = \sum_i s_i $$
  - Example: 62 frames, process A (10 KB), process B (127 KB), $S = 137\ \text{KB}$:
    - A: $10/137 \times 62 \approx 4$ frames
    - B: $127/137 \times 62 \approx 57$ frames
  - **Priority-based allocation:** replace size with priority weights.

### 5.3 Global vs Local Replacement

When a process page-faults and no free frame is available, the OS must decide where to look for a victim.

- **Global replacement:** the victim is chosen from **all** frames in the system. The faulting process can take a frame from another process.
  - Overall **throughput** is better, so it is generally used in modern OSes.
  - **Downside:** a process cannot predict its own page-fault rate, because other processes may silently take frames from its working set.
- **Local replacement:** the victim is chosen only from the **faulting process's own** frames.
  - The per-process page-fault rate is predictable.
  - **Downside:** if one process has too few frames, it thrashes alone even while other processes have surplus, and available memory may go underused.

### 5.4 Reclaiming Pages and the Reaper

Practical global replacement uses a **reaper** routine that runs in the background:

- It is triggered when the free-frame list drops below a **minimum threshold**.
- It walks pages using an LRU-approximation algorithm and reclaims them.
- It stops when free memory rises above a **maximum threshold**.

When free memory is critically low and the reaper cannot keep up, **Linux runs the OOM Killer** (Out-Of-Memory Killer), which terminates the process with the highest OOM score. The OOM score rises mainly with the percentage of memory a process uses, and depending on the implementation it also combines factors such as running time or an adjustment value for how killable the process is.

![Figure 14. Page reclamation by the reaper between the minimum and maximum thresholds](../../images/Week12_p29.png)

*Figure 14. Page reclamation by the reaper between the minimum and maximum thresholds*

### 5.5 NUMA and Frame Allocation

On **NUMA (Non-Uniform Memory Access)** machines, each CPU has its own local memory bank, and accessing local memory is significantly faster than accessing memory attached to another CPU.

- On a page fault, the OS should allocate a frame on the NUMA node **closest to the CPU** that is running the faulting process.
- Linux maintains a separate free-frame list per NUMA node.
- The scheduler tries to keep a process on the CPU it most recently ran on, improving both cache reuse and locality of physical memory.

### 5.6 Thrashing

**Thrashing** is the state in which a process spends more time handling page faults than executing real instructions.

**Vicious cycle:**

```text
1. Process has too few frames for its current working set
2. Page faults happen on nearly every instruction
3. I/O surges; the process is mostly blocked
4. CPU utilization drops (everyone is waiting on I/O)
5. The OS sees low CPU utilization and SCHEDULES MORE PROCESSES
6. Now every process has even fewer frames
7. Go to step 2 — faults explode further
```

*Figure 15. Vicious cycle of thrashing*

**Symptom plot:** as the degree of multiprogramming rises, CPU utilization first climbs, then **suddenly collapses** past a critical point. That collapse is thrashing.

![Figure 16. CPU utilization and thrashing by degree of multiprogramming](../../images/Week12_p32.png)

*Figure 16. CPU utilization and thrashing by degree of multiprogramming*

> **Note:** **The canonical OS pathology.** Thrashing is what happens when the abstraction breaks. The whole point of virtual memory is that programs can be larger than RAM, but only if their active footprint fits in the available frames. Push that footprint past the available frames and the system spends more time shuffling pages than computing.

**Defense:** preferring **local replacement** keeps one process's bad behavior from spreading to the others. A better approach is to prevent over-commitment in the first place using working sets or PFF (below).

### 5.7 Locality Model

A process **moves from locality to locality** during execution:

- A **locality** is a set of pages used intensively over some interval.
- Function calls define new localities: the function's code, its local variables, and the globals it touches.
- Returning from a function exits that locality.
- Localities can overlap (the caller's data is still in frequent use after the callee returns).

**Implication:** if a process gets enough frames for its **current** locality, faults occur only during the brief transitions between localities and almost never during steady-state execution. If the locality does not fit, the process thrashes.

### 5.8 Working-Set Model

The working-set model is a **practical** approximation of the current locality. It defines a working-set window $\Delta$ as a number of recent memory references.

$$
WS_i(t) = \{\text{pages referenced by process } i \text{ in the last } \Delta \text{ references}\}
$$

- If $\Delta$ is too small, it does not capture the whole locality, which causes needless misses.
- If $\Delta$ is too large, it spans multiple localities, which causes over-allocation.
- If $\Delta = \infty$, it covers every page the process ever touched.

**Example:** $\Delta = 10$

```text
...2 6 1 5 7 7 7 7 5 1 | 6 2 3 4 1 2 | 3 4 4 4 3 4 3 4 4 4 | 1 3 2 3 4...
   └──── Δ = 10 ─────┘t1               └──── Δ = 10 ─────┘t2

WS(t1) = {1, 2, 5, 6, 7}   WS(t2) = {3, 4}
```

*Figure 17. Working sets at two moments with a window of 10*

![Figure 18. Working-set model](../../images/Week12_p34.png)

*Figure 18. Working-set model*

**Thrashing prevention with WS:**
- $WSS_i = |WS_i|$ is the number of frames needed by process $i$.
- $D = \sum_i WSS_i$ is the total frame demand of the system.
- If $D > m$ (frames available), **thrashing is imminent**.
  - **Solution:** suspend one process to free its frames.
- If $D \leq m$, the unused frames can support a new process.

### 5.9 Working-Set Implementation

Tracking the working set exactly is expensive, so the typical approximation uses **timer interrupts and reference bits**:

```text
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

More history bits give finer accuracy but more bookkeeping per timer interrupt. As always with approximation, the right setting is a workload-dependent compromise.

### 5.10 Page-Fault Frequency (PFF)

The **PFF** strategy controls thrashing **directly** by monitoring the page-fault rate of each process:

| Situation                                        | Action                              |
|--------------------------------------------------|-------------------------------------|
| Process's PFF > upper bound                      | **Allocate more frames** to it      |
| Process's PFF < lower bound                      | **Reclaim frames** from it          |
| PFF > upper bound but no free frames in system   | **Suspend the process**             |

*Table 9. Actions of the PFF strategy by situation*

Compared to the working-set model:
- It is **simpler to implement**, because it only counts faults.
- It **directly controls** what we ultimately care about (the fault rate).
- It is less informed about **why** the rate is what it is.

![Figure 19. Page-fault frequency controlled by upper and lower bounds](../../images/Week12_p36.png)

*Figure 19. Page-fault frequency controlled by upper and lower bounds*

---

<br>

## 6. Memory Compression

Memory compression is an alternative to writing dirty pages out to disk: it **compresses them in RAM**.

- Several pages are collected, run through a compressor (Apple WKdm, Microsoft Xpress), and the **compressed forms are packed into a single frame**.
- It trades some CPU for a lot less disk I/O. On modern hardware, compression is often faster than SSD paging, and on mobile devices with limited or no swap, it is essentially the only option.

```text
Before compression                     After compression
modified-frame list: [F12][F45][F88]   F99 = compressed pack of pages F12, F45, F88
free-frame list:     [F99][F07]        free-frame list: [F07][F12][F45][F88]
```

*Figure 20. Compressing three modified pages into one frame and returning the original frames*

![Figure 21. Free-frame list and modified-frame list before compression](../../images/Week12_p37.png)

*Figure 21. Free-frame list and modified-frame list before compression*

![Figure 22. Free-frame list after compression](../../images/Week12_p38.png)

*Figure 22. Free-frame list after compression*

When a compressed page is referenced again, the OS decompresses it into a fresh frame. This costs a page fault and a decompression step, but no disk I/O.

**macOS, iOS, Android, and Windows 10 and later** use memory compression. Compressed pages typically take 30-50% of the original size, so a 4 KB page becomes about 1.2-2 KB.

---

<br>

## 7. Kernel Memory Allocation

### 7.1 Why Kernel Memory Is Special

The kernel cannot simply use the paging-based allocator it gives to user processes. There are two reasons:

1. **Many tiny objects.** Kernel structures (PCBs, file objects, semaphores, network packets) are often **smaller than a page**. Allocating a whole frame per object would waste 90% or more of memory for small objects.
2. **Some hardware demands physically contiguous memory**, DMA controllers in particular. Page-level allocation gives you pages, but not necessarily **contiguous** pages.

Two classical strategies coexist in real kernels:

- **Buddy system:** coarse-grained contiguous allocation with fast coalescing.
- **Slab allocation:** fine-grained, per-object-type caches.

### 7.2 Buddy System

Memory is managed in **power-of-two-sized blocks** within a contiguous physical region.

- A request of size $r$ is rounded up to the next power of 2.
  - Example: a 21 KB request gets a 32 KB block.
- If no block of that size exists, a larger one is recursively split in half (each half being the buddy of the other) until the right size is reached.

```text
Allocating 21 KB from a 256 KB segment:

   [           256 KB             ]
   [   128 KB AL  ][   128 KB AR  ]   ← split
   [ 64 BL ][ 64 BR ][   128 KB   ]   ← split AL
   [32 CL][32 CR][ 64 BR ]...         ← split BL — 32 KB allocated for 21 KB request
```

*Figure 23. Splitting a 256 KB segment to allocate a 32 KB block for a 21 KB request*

![Figure 24. Block splitting in the buddy system](../../images/Week12_p40a.png)

*Figure 24. Block splitting in the buddy system*

**Deallocation triggers coalescing.** When a block is freed and its buddy is also free, the two merge back into the next larger size, recursively up the tree.

- **Pro:** coalescing is simple and fast (constant time per level, $O(\log n)$ total).
- **Con:** **internal fragmentation**. A 33 KB request still occupies a 64 KB block (about 50% waste).

### 7.3 Slab Allocation

**Per-object-type caches** keep pre-allocated, ready-to-use instances of a particular kernel data structure.

- A **slab** is one or more contiguous physical pages.
- A **cache** is a collection of slabs that all hold instances of a specific object type (e.g., PCB cache, file-object cache, semaphore cache).
- Each cache has its own slabs and its own free list of objects.

```text
Cache: "PCB"   ─── slab 1: [PCB][PCB][PCB][PCB]
                ── slab 2: [PCB][PCB][PCB][PCB]
                ── ...
```

*Figure 25. PCB cache made of several slabs*

![Figure 26. Relationship among kernel objects, caches, and slabs](../../images/Week12_p40b.png)

*Figure 26. Relationship among kernel objects, caches, and slabs*

**Operations:**
- Allocating grabs the first free object from the cache.
- Freeing returns the object to the cache's free list (no destruction, no re-initialization).

**Slab states:**
- **Full:** every object in the slab is in use.
- **Partial:** used and free objects are mixed.
- **Empty:** every object is free.

The allocator prefers a **partial slab, then an empty slab, then a newly allocated slab**.

**Advantages:**
- **No internal fragmentation.** The slot size exactly matches the object size.
- **Fast.** It reuses pre-initialized objects without the cost of general-purpose allocation and freeing.
- **Cache-friendly.** Objects of the same type are clustered together.

Linux variants:
- **SLAB:** the original Linux implementation.
- **SLOB:** a minimal list-based implementation for memory-constrained embedded systems.
- **SLUB:** the default since Linux 2.6.24, with less metadata overhead and better SMP scaling.

---

<br>

## 8. Other Considerations

### 8.1 Prepaging

Pure demand paging suffers a flood of faults at process startup. **Prepaging** preloads pages that are likely to be needed soon:

- On process suspension, remember the working set.
- On resume, load all those pages **before** the first instruction executes.

**Cost and benefit:** suppose $s$ pages are prepaged and a fraction $\alpha$ of them end up actually used.
- Benefit: $s \cdot \alpha$ faults are avoided.
- Waste: $s \cdot (1 - \alpha)$ pages that are never used are loaded.
- Prepaging pays off when $\alpha$ is close to 1.

Linux's **`readahead()`** is a related mechanism: it explicitly tells the kernel that a range of a file is about to be read, so the kernel should read it into memory in advance.

### 8.2 Program Structure and Locality

Programmer-controlled locality can outweigh any OS optimization. Consider a 128 × 128 `int` array with a page size of 128 words (128 `int`s), so that one row exactly fills one page:

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

Both fault counts assume that fewer than 128 frames are allocated to the whole program. Column-major access touches all 128 pages in every outer iteration, so every access faults and the total is 128 × 128 = 16,384 faults; row-major access fills one row (one page) before moving to the next, so it incurs only 128 faults.

Data-structure locality matters too:
- A **stack** has excellent locality, because it always touches the top.
- A **hash table** has poor locality, because its references scatter by design.

### 8.3 I/O Interlock and Page Locking

Global replacement combined with concurrent I/O causes a subtle problem:

```text
Process A: starts a disk READ into buffer page X of process A
Process B: page-faults; OS picks page X as the victim (global replacement!)
         OS rewrites page X with process B's new contents
DMA completes: writes the original disk read into the (now hijacked) page X
              → corruption!
```

*Figure 27. Global replacement hijacking the buffer page of an in-progress DMA*

**Solutions:**

- **Indirect I/O via kernel buffers:** always read into kernel memory first, then copy to user memory. This is safe but adds a copy.
- **Lock bit (pin):** set a bit meaning the frame is locked and must not be replaced. The page-replacement algorithm skips locked frames, and the lock is released when the I/O completes. Kernel pages are typically pinned permanently for the same reason.

---

<br>

## 9. Lab: Page-Replacement Algorithm Simulator

**Goal:** simulate **FIFO, LRU, and OPT** on the same reference string with the same number of frames and compare page-fault counts.

**Requirements:**
- The **input** is a reference string (integer array) and a frame count.
- The **output for each algorithm** is the frame state after each reference, with `* Fault` marked when applicable.
- The **final output** is a comparison table.

**Input example:**
```text
Reference String: 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
Number of Frames: 3
```

**Core FIFO sketch in Python:**

```python
def fifo(ref_string, num_frames):
    frames = []
    faults = 0
    for page in ref_string:
        fault = page not in frames     # decide BEFORE mutating `frames`
        if fault:
            faults += 1
            if len(frames) < num_frames:
                frames.append(page)
            else:
                frames.pop(0)          # evict oldest
                frames.append(page)
        print(f"Ref: {page}  Frames: {frames}  {'* Fault' if fault else ''}")
    return faults
```

**Expected output:**

```text
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

> **Note:** The sketch keeps frames in queue order (`pop(0)` followed by `append`) and does not print empty slots, so its output format differs from the expected output. For example, after reference 2 the sketch prints `[0, 1, 2]`, whereas the expected output keeps fixed frame slots like Table 1 and shows `[2, 0, 1]`. To match the expected format, put the new page into the victim's slot (`frames[frames.index(victim)] = page`), fill empty slots with `-`, and print the fault mark as `* Page Fault`. The fault count of 15 is the same either way.

---

<br>

## Summary

| Concept | Key Summary |
|-------|-------------|
| Virtual memory | It decouples the logical from the physical address space so that programs may exceed RAM, and it is the foundation that makes the other OS abstractions possible. |
| Demand paging | A lazy pager loads pages only on reference, implemented with the valid/invalid bit and the page-fault trap. |
| EAT | $(1-p)\cdot \text{mem} + p\cdot \text{pf}$ is brutally sensitive to the fault rate, so $p$ must be in the parts-per-million range (at most $2.5 \times 10^{-6}$ in the example of Section 1.10), and every later technique exists to keep $p$ near zero. |
| Copy-on-Write | `fork()` shares pages and copies only on write, while `vfork()` skips copying entirely. |
| Page replacement | FIFO, OPT, and LRU are the basic algorithms, the dirty bit lets clean pages skip the write-back, and page replacement is what keeps the virtual-memory abstraction believable. |
| Hierarchy of replacement algorithms | OPT is the ceiling, LRU is the practical ideal, and clock is what real systems use; this hierarchy of approximations is a direct response to hardware constraints. |
| Belady's anomaly | FIFO can fault more with more frames, but this is FIFO's fault rather than virtual memory's, and stack algorithms (LRU, OPT) eliminate it by construction. |
| LRU approximation | The reference bit and the clock approximate LRU, and the enhanced clock uses (R, M) classes. |
| Frame allocation | Allocation can be equal or proportional, and the minimum number of frames comes from the instruction architecture. |
| Replacement scope | Global replacement gives high throughput but is unpredictable, while local replacement is predictable but may underuse memory. |
| Locality | Locality is the engine that makes virtual memory work, and without it every replacement algorithm degrades to random replacement. |
| Thrashing | Too few frames cause a fault explosion and a collapse of CPU utilization, which is the failure mode of virtual memory. |
| Working-set, PFF | They track recent references or the fault rate and exist specifically to detect and prevent thrashing. |
| Memory compression | Pages are compressed in RAM instead of being swapped to disk. |
| Kernel memory | Because the kernel allocates many tiny structures and sometimes needs physically contiguous memory, the buddy system (powers of 2, contiguous, easy coalescing) plus slab (per-object caches) is the textbook answer. |
| Other | Prepaging, program-structure locality, and page pinning for I/O also affect performance and correctness. |

---

<br>

## Self-Check Questions

1. **Definition:** What does it mean to separate logical memory from physical memory, and what does that buy you?

   <details>
   <summary>Answer</summary>

   Logical addresses live in each process's private virtual address space; physical addresses live in the shared DRAM. A page table maps the former to the latter, but **not every logical page needs to be mapped** at a given moment; absent pages are simply marked invalid in the PTE. This decoupling lets the OS (1) run programs larger than physical RAM by bringing pages in on demand, (2) keep many processes resident simultaneously, (3) start a process before it is fully loaded, (4) share physical frames between processes, and (5) implement a fast `fork()` via copy-on-write. None of these is possible without the indirection.

   </details>

2. **Page-fault walkthrough:** A process executes `MOV [0x4000], EAX`. The page containing 0x4000 is marked invalid. Walk through every step from the CPU to the instruction restart.

   <details>
   <summary>Answer</summary>

   (1) The CPU forms the effective address 0x4000 and presents it to the MMU. (2) The MMU walks the page table, finds that the PTE has valid bit = 0, and raises a **page-fault trap**. (3) The kernel trap handler examines the faulting address and the PTE. (4) It distinguishes an illegal address (deliver SIGSEGV) from a valid page on disk; here, the page is valid but on disk. (5) The kernel pulls a free frame from the free-frame list. If the list is empty, page replacement picks a victim, although the reaper normally keeps the list stocked. (6) It issues a disk I/O to read the page into that frame, and the process blocks waiting on the I/O. (7) On I/O completion, the kernel writes (frame number, valid=1, permissions) into the PTE. (8) It returns from the trap, **restarting** the faulting `MOV` instruction. (9) The MMU translates 0x4000 again (now valid), and the store succeeds.

   </details>

3. **EAT impact:** With mem = 100 ns and pf = 5 ms, what page-fault rate keeps the slowdown within 5%?

   <details>
   <summary>Answer</summary>

   EAT = $(1-p)\cdot 100 + p \cdot 5{,}000{,}000$ = $100 + 4{,}999{,}900 p$. A slowdown within 5% means EAT ≤ 105, so $5 \geq 4{,}999{,}900 p$, giving $p \leq 10^{-6}$, which is about **one fault per million memory accesses**. A spinning-disk page fault is so expensive that the fault rate must be in the parts-per-million range. SSDs relax this condition: with pf of about 50 μs, the same 5% target gives $p \leq 5/49{,}900 \approx 10^{-4}$.

   </details>

4. **Copy-on-write trace:** A parent has pages P1 (code, read-only) and P2 (data, writable). The parent calls `fork()`, and the child then writes to P2. Describe the frame and PTE state at each step.

   <details>
   <summary>Answer</summary>

   **Before fork:** the parent's PTEs point to frames F1 (P1, RO) and F2 (P2, RW). **fork():** the kernel copies the parent's PTEs into the child's page table. Both processes now have PTEs pointing at F1 (still RO; code never needs COW) and F2, which the kernel **marks RO in both PTEs** even though P2 is logically writable. **Child writes to P2:** the MMU sees the RO PTE and raises a **protection fault**. The kernel allocates a new frame F2', copies F2 into F2', and updates the child's PTE for P2 to point at F2' and be RW. The parent's PTE for P2 still points at F2, which is no longer shared, so it can be made RW again (depending on the implementation, the fault handler restores write permission without copying when the parent next tries to write). **Result:** F1 is still shared (one frame for code), and F2 and F2' now hold independent copies of the writable data, but only **because the child wrote**. Had the child called `exec()` instead, F2 would never have been duplicated.

   </details>

5. **FIFO vs LRU vs OPT:** Run all three on the reference string `1 2 3 4 1 2 5 1 2 3 4 5` with 4 frames. Tabulate the fault counts.

   <details>
   <summary>Answer</summary>

   With 4 frames, **FIFO has 10 faults** (per the trace in Section 3.6). **LRU has 8 faults**: after loading 1, 2, 3, and 4, the misses on 5, 3, 4, and 5 evict 3, 4, 5, and 1 in turn. **OPT has 6 faults**. Its first replacement evicts 4, whose next use is farthest away, and its second replacement evicts a page that is never used again. Thus FIFO (10) > LRU (8) > OPT (6). FIFO also shows Belady's anomaly here: it has 9 faults with 3 frames and 10 with 4, while the fault counts of LRU (10 with 3 frames, 8 with 4) and OPT (7 with 3 frames, 6 with 4) decrease as frames are added.

   </details>

6. **Belady's anomaly:** Why does FIFO suffer from it while LRU does not? Use the stack-algorithm property to justify.

   <details>
   <summary>Answer</summary>

   A replacement algorithm is a **stack algorithm** if, for every prefix of any reference string, the resident set with $n$ frames is a subset of the resident set with $n+1$ frames. **LRU** trivially satisfies this: with $n$ frames the resident set is the $n$ most recently referenced pages, which is a subset of the $n+1$ most recently referenced pages. Adding a frame can only add to the resident set and never changes which other pages are kept, so the number of page faults can only stay the same or decrease. **FIFO** is **not** a stack algorithm: the resident set with $n$ frames depends on insertion order, and with $n+1$ frames the order shifts so that a page resident in the $n$-frame run may **not** be in the $n+1$-frame resident set. Different eviction patterns make Belady's anomaly possible.

   </details>

7. **Reference bit and clock:** Five pages sit in a circular queue with reference bits (1, 1, 0, 1, 0). The hand points at page 1, and one page must be evicted. Walk the clock pointer and state which page is evicted.

   <details>
   <summary>Answer</summary>

   Starting at page 1, its bit is 1, so it is cleared to 0 and the hand advances. Page 2 also has bit 1, so it is cleared to 0 and the hand advances. Page 3 has bit 0, so it is the **victim** and is evicted. Pages 1 and 2 survive after receiving second chances, and their bits are now 0, so they become the next victims if they are not referenced again in the meantime. Pages 4 and 5 are untouched in this pass. The bits just before insertion are (0, 0, ?, 1, 0), and the new page takes page 3's slot. Depending on the implementation, the new page's reference bit starts at 0 or 1, and the hardware sets it to 1 when the restarted instruction accesses the page.

   </details>

8. **Enhanced second-chance:** A frame is (R=0, M=1). What does this say about the page, and what is the eviction cost compared to (R=0, M=0)?

   <details>
   <summary>Answer</summary>

   R=0 means the page has not been referenced in the current cycle (not recently used); M=1 means it has been written to since it was loaded (dirty). So this page is **stale but modified**. Compared to (R=0, M=0), which is also stale but **clean**, evicting (0, 1) is more expensive, because the dirty contents must be **written back to swap or disk** before the frame can be reused. The clean page can simply be dropped, because the disk already has the same bytes. The enhanced second-chance algorithm evicts (0, 0) first precisely to avoid the write-back, evicting (0, 1) only when no clean stale page is available.

   </details>

9. **Working set:** With $\Delta = 8$ and the reference string `1 2 1 3 4 5 1 2 3 4 5 6`, what is the working set right after the last reference?

   <details>
   <summary>Answer</summary>

   The last 8 references are positions 5-12, `4, 5, 1, 2, 3, 4, 5, 6`. So $WS = \{1, 2, 3, 4, 5, 6\}$: six distinct pages in the last 8 references. The process needs at least 6 frames to avoid thrashing in its current locality.

   </details>

10. **Thrashing diagnosis:** After the OS admits one more process, a server's CPU utilization drops from 80% to 15% and the page-fault rate spikes. What is happening, and what is the fix under (a) global replacement, (b) local replacement, and (c) PFF?

    <details>
    <summary>Answer</summary>

    This is the classic onset of thrashing. The total working-set demand $D = \sum WSS_i$ now exceeds the number of physical frames $m$, so every process is missing pages, page-fault I/O dominates, and the CPU idles waiting on disk. **(a) Global replacement:** the OS should **suspend** one process (typically the most recently admitted one, or the one with the highest fault rate) until $D \leq m$; suspending it frees its frames for the others. **(b) Local replacement:** local replacement already keeps one bad process from spreading its behavior to others, but here the admission control failed. The fix is not to admit the new process if $\sum WSS_i + WSS_\text{new} > m$. **(c) PFF:** the page-fault rate of one or more processes exceeds the upper bound. The algorithm tries to give them more frames, fails because no frames are free, and then suspends a process. The outcome is the same; the signal is simply measured directly rather than inferred from working-set sizes.

    </details>

11. **Buddy vs slab:** Why does the kernel need **both** allocators? What is each good and bad at?

    <details>
    <summary>Answer</summary>

    **Buddy** handles **coarse-grained, contiguous physical memory** such as pages and multi-page blocks. It coalesces freed buddies in $O(\log n)$, so fragmentation of large blocks recovers cleanly. But it suffers heavy **internal fragmentation** for sub-page objects (a 17-byte allocation costs a 32-byte block, or worse, the page-sized minimum), which is unacceptable for the millions of tiny kernel structures (PCBs, file objects, inodes). **Slab** complements buddy by sitting **on top of** it: a slab is one or more contiguous pages obtained from buddy and divided into fixed-size object slots. Each slab cache is dedicated to one type, so the slot size exactly matches the object size, giving **no internal fragmentation** for those objects, and allocation and freeing are O(1) free-list pops and pushes. Slab is bad at large allocations of varying sizes (it would need a separate cache per size), which buddy handles naturally. So the kernel uses **buddy** for page-frame allocation and large contiguous blocks (e.g., DMA buffers), and **slab** for frequently used typed objects.

    </details>

12. **Program structure:** On a system with 4 KB pages, consider `double M[1024][1024]` (each row is 8 KB, that is, 2 pages). Compare the page-fault behavior of `for i, for j: M[i][j] = 0` and `for j, for i: M[i][j] = 0`.

    <details>
    <summary>Answer</summary>

    **Row-major access (`for i, for j`)** writes adjacent elements in turn, so it visits the two pages of each row in sequence. The 1024 rows occupy 2048 pages, so regardless of the frame count this order incurs **2048 page faults**, one per distinct page, and it cannot do better. **Column-major access (`for j, for i`)** visits a different row, and therefore a different page, at each inner-loop step. One outer-loop iteration touches one page per row, the page that holds column j, for a total of 1024 pages. Assuming LRU, with at least 1024 frames the first page of each row stays resident while j = 0-511, and the second pages take over from j = 512, so the total is again 2048 faults. With fewer than 1024 frames, pages are evicted before the next outer iteration, potentially causing up to **1024 × 1024 ≈ $10^6$ faults**, about **500×** more. The example of Section 8.2 (16,384 versus 128 faults) rests on the same assumption that the array does not fit in the available frames. For this layout, C's row-major order is always optimal.

    </details>
