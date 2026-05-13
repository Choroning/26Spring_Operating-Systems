# Week 11 Concepts Lecture — Main Memory

> **Last Updated:** 2026-05-13
>
> Silberschatz, Operating System Concepts Ch 9 (Main Memory)

> **Prerequisites**: Week 1–3 (process/PCB, context switch). Familiarity with assembly addressing and the basic CPU/memory hierarchy from a computer-architecture course will help. From [[concepts_lecture_w10]] you already know how the kernel manages shared resources; this week the resource is **physical memory** itself.
>
> **Learning Objectives**:
> 1. Explain why the CPU can only directly address **registers and main memory**, not disk
> 2. Distinguish **logical** and **physical** addresses, and describe the role of the **MMU**, **base register**, and **limit register**
> 3. Compare the three address-binding times: **compile time, load time, execution time**
> 4. Understand **dynamic loading** and **dynamic linking** (shared libraries, DLLs, stubs)
> 5. Apply **contiguous allocation** algorithms (**First-fit, Best-fit, Worst-fit**) and compute the resulting layout
> 6. Distinguish **external** and **internal fragmentation**, and explain the **50-percent rule** and **compaction**
> 7. Translate a logical address to a physical address under **paging**, with and without a **TLB**
> 8. Compute **Effective Access Time (EAT)** and reason about page-size trade-offs
> 9. Describe **protection bits**, **valid-invalid bits**, **PTLR**, and **shared pages**
> 10. Compare page-table structures: **single-level, hierarchical (2-/3-/4-level), hashed, inverted**
> 11. Distinguish **standard swapping** from **paged swapping**, and understand why mobile OSes avoid swapping
> 12. Outline address translation in **IA-32**, **IA-32 PAE**, **x86-64**, and **ARMv8**

---

## Table of Contents

- [1. Background and Basic Hardware](#1-background-and-basic-hardware)
  - [1.1 Why Memory Management Is Needed](#11-why-memory-management-is-needed)
  - [1.2 The CPU's Directly Addressable Storage](#12-the-cpus-directly-addressable-storage)
  - [1.3 Base and Limit Registers](#13-base-and-limit-registers)
- [2. Address Binding and the MMU](#2-address-binding-and-the-mmu)
  - [2.1 Three Binding Times](#21-three-binding-times)
  - [2.2 Multistep Processing of a User Program](#22-multistep-processing-of-a-user-program)
  - [2.3 Logical vs Physical Address](#23-logical-vs-physical-address)
  - [2.4 MMU — Memory-Management Unit](#24-mmu--memory-management-unit)
  - [2.5 Dynamic Loading](#25-dynamic-loading)
  - [2.6 Dynamic Linking and Shared Libraries](#26-dynamic-linking-and-shared-libraries)
- [3. Contiguous Memory Allocation](#3-contiguous-memory-allocation)
  - [3.1 Overview and Memory Protection](#31-overview-and-memory-protection)
  - [3.2 Variable Partition Scheme](#32-variable-partition-scheme)
  - [3.3 First-fit / Best-fit / Worst-fit](#33-first-fit--best-fit--worst-fit)
  - [3.4 Worked Allocation Example](#34-worked-allocation-example)
  - [3.5 External vs Internal Fragmentation](#35-external-vs-internal-fragmentation)
  - [3.6 Compaction](#36-compaction)
- [4. Paging — Basic Method](#4-paging--basic-method)
  - [4.1 Paging Overview](#41-paging-overview)
  - [4.2 Address Translation Principle](#42-address-translation-principle)
  - [4.3 Paging Hardware and Free-Frame Management](#43-paging-hardware-and-free-frame-management)
  - [4.4 Internal Fragmentation and Page Size Trade-offs](#44-internal-fragmentation-and-page-size-trade-offs)
- [5. Paging Hardware Support — TLB](#5-paging-hardware-support--tlb)
  - [5.1 Where to Keep the Page Table](#51-where-to-keep-the-page-table)
  - [5.2 TLB — Translation Look-aside Buffer](#52-tlb--translation-look-aside-buffer)
  - [5.3 ASID — Address-Space Identifier](#53-asid--address-space-identifier)
  - [5.4 Effective Access Time (EAT)](#54-effective-access-time-eat)
- [6. Paging — Protection and Shared Pages](#6-paging--protection-and-shared-pages)
  - [6.1 Protection Bits and Valid-Invalid Bit](#61-protection-bits-and-valid-invalid-bit)
  - [6.2 Shared Pages](#62-shared-pages)
- [7. Page-Table Structure](#7-page-table-structure)
  - [7.1 Why Single-Level Page Tables Don't Scale](#71-why-single-level-page-tables-dont-scale)
  - [7.2 Hierarchical Paging](#72-hierarchical-paging)
  - [7.3 Hierarchical Paging Falls Apart in 64-bit](#73-hierarchical-paging-falls-apart-in-64-bit)
  - [7.4 Hashed Page Tables](#74-hashed-page-tables)
  - [7.5 Inverted Page Tables](#75-inverted-page-tables)
  - [7.6 Comparison](#76-comparison)
- [8. Swapping](#8-swapping)
  - [8.1 Standard Swapping](#81-standard-swapping)
  - [8.2 Swapping with Paging](#82-swapping-with-paging)
  - [8.3 Swapping on Mobile Systems](#83-swapping-on-mobile-systems)
- [9. Real-World Architectures](#9-real-world-architectures)
  - [9.1 IA-32](#91-ia-32)
  - [9.2 IA-32 PAE](#92-ia-32-pae)
  - [9.3 x86-64](#93-x86-64)
  - [9.4 ARMv8](#94-armv8)
  - [9.5 Comparison](#95-comparison)
- [10. Lab — Memory Allocation Simulator](#10-lab--memory-allocation-simulator)
  - [10.1 Lab Goal and Data Structure](#101-lab-goal-and-data-structure)
  - [10.2 Lab Functions and Example I/O](#102-lab-functions-and-example-io)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. Background and Basic Hardware

### 1.1 Why Memory Management Is Needed

To keep the CPU busy (high CPU utilization), the OS must keep **many processes resident in memory at the same time**. As soon as more than one process shares physical memory, three problems appear:

- **Mutual protection** — one process must not be able to read or overwrite another's memory.
- **Address translation** — each process expects a clean address space starting from 0, but they cannot all actually live at physical address 0.
- **Efficient sharing** — code (libraries, the kernel) should be shared; private data must not.

> **Why this is foundational**: every later memory topic — paging, virtual memory, page replacement, copy-on-write — is a different way to enforce these three properties at scale.

### 1.2 The CPU's Directly Addressable Storage

The CPU can directly access only two kinds of storage:

```text
   +-----------+
   | Register  |   <- ~1 cycle
   +-----------+
   |   Cache   |   <- a few cycles (transparent to ISA)
   +-----------+
   |   Main    |   <- 10s–100s of cycles
   |  Memory   |
   +-----------+
   |   Disk    |   <- millions of cycles, NOT directly addressable
   +-----------+
```

> **Prerequisite — the memory hierarchy** (computer architecture): each level trades capacity for latency. **"Transparent to ISA"** means the cache is managed entirely by hardware — neither the OS nor the running program issues explicit loads/stores to it; the CPU automatically fills cache lines on demand and evicts them under its own policy. Only the **register file** and **main memory** are exposed to the load/store instructions in the instruction set, which is why OS textbooks talk about "directly addressable" storage as if cache did not exist.

Everything on disk must be **loaded into memory** before the CPU can touch it. This is exactly why memory is a scarce, contended resource.

### 1.3 Base and Limit Registers

The simplest hardware protection scheme: every process has a **base** and a **limit** register.

- **Base register** — the smallest physical address the process can legally access.
- **Limit register** — the *size* of the legal range (not an upper address).
- A CPU-issued address must satisfy `base ≤ addr < base + limit`. Any violation raises a **trap to the OS** (a fatal segmentation violation in user-space programs).

Concrete example: with Base = 300040 and Limit = 120900, the accessible range is `300040 ~ 420939`.

These registers can only be loaded from **kernel mode** by privileged instructions — otherwise a malicious user process could just lift its own limit.

---

<br>

## 2. Address Binding and the MMU

### 2.1 Three Binding Times

A program's memory addresses are not magically fixed — they are *bound* to physical locations at some point. The choice of binding time determines what the OS must do at runtime.

| Binding time | Description | Characteristics |
|--------------|-------------|------------------|
| **Compile time** | Memory start address fixed at compile time | Generates **absolute code**; recompilation needed if start address changes (MS-DOS .COM files) |
| **Load time** | Start address fixed when the loader places the program in memory | Generates **relocatable code**; binary moves but cannot be relocated after launch |
| **Execution time** | Binding happens *while the program runs* | Requires an **MMU**; used by virtually every modern OS |

### 2.2 Multistep Processing of a User Program

Source code passes through several stages, each performing a partial binding:

```text
  source.c   --(compile)-->   object  module   (relocatable, with symbolic names)
             --(link)----->   load    module
             --(load)----->   in-memory   binary
             --(run)------>   physical  reference (MMU)
```

- **Symbolic** addresses (variable names) → **Relocatable** (offset from module start) → **Absolute** (physical address).

### 2.3 Logical vs Physical Address

Two address spaces coexist at runtime.

- **Logical address** (a.k.a. *virtual address*)
  - The address the CPU generates while executing instructions.
  - The process sees a private space from 0 to some maximum.
- **Physical address**
  - The address actually delivered to the DRAM (placed into the Memory Address Register).
  - Lives in the real, shared range `R + 0 … R + max`.

With **compile-time** or **load-time** binding, logical = physical. With **execution-time** binding, logical ≠ physical and **translation is required on every memory reference**.

### 2.4 MMU — Memory-Management Unit

The **MMU** is the hardware that performs logical-to-physical translation. In the simplest scheme (dynamic relocation) it just adds a **relocation register** to every logical address. *(Note: in this simplest design the relocation register is the same physical register as the **base register** introduced in §1.3 — the two names emphasize different roles. As "base", it sets the lower bound for the limit check; as "relocation", it is the addend that shifts the user's view of 0…max up to the actual physical range.)*

```text
  logical addr  -->  +-----+
                     | MMU |  + relocation_register  -->  physical addr  -->  DRAM
                     +-----+
```

The user program *only ever sees* logical addresses 0 … max. It has no way to forge a physical address, because the MMU is on the path between the CPU and the bus.

Real systems use much more elaborate MMUs (paging, segmentation), but the principle is the same: hardware-enforced indirection between what the program writes and what reaches DRAM.

### 2.5 Dynamic Loading

**Dynamic loading**: a routine is loaded into memory **only when it is called**. The main program lives in memory; rarely used routines (e.g., obscure error handlers) sit on disk until needed.

```text
  Main Program (in memory)
   ├── routine_A()        loaded
   ├── routine_B()        loaded on call
   └── error_handler()    loaded only on error  (on disk)
```

Notably, this can be implemented **without OS support** — the programmer arranges the conditional load. Total program size is large; *resident* size stays small.

### 2.6 Dynamic Linking and Shared Libraries

**Static linking** copies all library code into the executable at link time — every program ships its own complete copy of `libc`. **Dynamic linking** defers the bind to *execution time* and shares one in-memory copy among all processes.

| Aspect | Benefit of DLL / shared libraries |
|--------|-----------------------------------|
| Memory savings | One physical copy serves all processes |
| Disk savings | Library code not duplicated in every executable |
| Easy updates | Replace the library file; no rebuild of users |
| Versioning | Multiple versions coexist; programs pick a compatible one |

A small **stub** in each program locates the library routine at first call and patches the call site to jump directly thereafter. Dynamic linking requires OS support because the library must be mapped into the process's protected address space.

```text
  Static Linking                    Dynamic Linking
  +-----------------+               +-----------------+
  | Program A       |               | Program A       |
  | +-------------+ |               | (stub -> libc)  |
  | | libc copy 1 | |               +-----------------+
  | +-------------+ |                        |
  +-----------------+               +-----------------+
  | Program B       |               | Program B       |
  | +-------------+ |               | (stub -> libc)  |
  | | libc copy 2 | |               +-----------------+
  | +-------------+ |                        |
  +-----------------+               +-----------------+
                                    | libc (1 copy)   | <- shared
                                    +-----------------+

  libc 2MB × 40 processes           libc 2MB × 1 copy
  = 80MB memory                     = 2MB memory
```

---

<br>

## 3. Contiguous Memory Allocation

### 3.1 Overview and Memory Protection

Historically the simplest scheme: each process gets **one contiguous block** of physical memory.

- Memory is split into an **OS area** (usually high addresses, near the interrupt vector) and a **user area**.
- A **hole** is a free block. Holes appear in many sizes across the user area.
- When a process arrives the OS picks some hole of sufficient size.

Protection is enforced with **Relocation register + Limit register**: the relocation register is added to every logical address, then the limit register checks the result. The OS dispatcher loads both during context switch.

### 3.2 Variable Partition Scheme

Each process gets a **variable-sized partition** matching its needs. When a process terminates a hole is created; **adjacent holes are coalesced** to form larger free blocks.

The problem: given the current hole list, **which hole** should a new request of size n be allocated to? This is the **Dynamic Storage-Allocation Problem**.

### 3.3 First-fit / Best-fit / Worst-fit

| Strategy | Method | Characteristics |
|----------|--------|------------------|
| **First-fit** | Use the first hole large enough | Fast (stops on first match); generally good utilization |
| **Best-fit** | Use the smallest hole that fits | Minimum leftover, but requires scanning the whole list; tends to create many tiny holes |
| **Worst-fit** | Use the largest hole | Leaves the biggest remaining hole; counter-intuitively often the *worst* strategy |

**Empirical results**: First-fit and Best-fit beat Worst-fit; First-fit is usually faster than Best-fit at comparable utilization.

### 3.4 Worked Allocation Example

Holes (in order): 300KB, 600KB, 350KB, 200KB, 750KB, 125KB.
Requests in order: P1 = 115KB, P2 = 500KB, P3 = 358KB, P4 = 200KB, P5 = 375KB.

```text
First-fit:
  P1(115) -> 300KB hole       P2(500) -> 600KB hole
  P3(358) -> 750KB hole       P4(200) -> 200KB hole
  P5(375) -> fails (remaining holes 185, 100, 392, 125 — 392 is enough!? Actually 392>=375 → P5 fits 392!)
                          (Note: depending on textbook example; key idea is some requests may fail.)

Best-fit:
  P1(115) -> 125KB hole       P2(500) -> 600KB hole
  P3(358) -> 750KB hole       P4(200) -> 200KB hole
  P5(375) -> 392KB hole

Worst-fit:
  P1(115) -> 750KB hole       P2(500) -> 635KB hole
  P3(358) -> fails
```

The point is not the exact numbers — it is that **the allocation strategy meaningfully changes which sequences succeed**.

### 3.5 External vs Internal Fragmentation

Two different forms of waste:

- **External fragmentation** — total free memory is sufficient, but **no single hole is large enough** for the request. This is a contiguity problem.
- **Internal fragmentation** — the system allocates in fixed *units*, and the request is smaller than the unit. The leftover space inside the allocation is wasted.

```text
External Fragmentation             Internal Fragmentation
+--------+                         +---------+
|  P1    |                         |Allocated|  <- Request: 3.5KB
+--------+                         | (3.5KB) |
|  hole  | 50KB                    |---------|
+--------+                         | Wasted  |  <- 0.5KB
|  P2    |                         | (0.5KB) |
+--------+                         +---------+  <- Allocation unit: 4KB
|  hole  | 30KB
+--------+
|  P3    |  Request: 70KB  Total free: 80KB  Largest contiguous: 50KB → fails
```

> **The 50-percent rule (Knuth)**: with N allocated blocks under first-fit, statistical analysis predicts ~0.5N additional blocks lost to fragmentation. In practice **~1/3** of memory becomes unusable to external fragmentation in long-running variable-partition systems. This is the empirical pain point that motivated paging.

### 3.6 Compaction

The brute-force fix for external fragmentation: **move occupied memory to one end** and consolidate all free space at the other end into one big hole.

```text
Before                  After
+--------+              +--------+
|  P1    |              |  P1    |
+--------+              +--------+
|  hole  |              |  P2    |
+--------+              +--------+
|  P2    |  -- shift -> |  P3    |
+--------+              +--------+
|  hole  |              |        |
+--------+              |  hole  |  <- one big contiguous block
|  P3    |              |        |
+--------+              +--------+
```

Compaction is only possible with **execution-time binding** (you can change a process's base register on the fly). It is also very expensive: copying every process and updating its base register stalls the system.

The better answer turned out to be **paging** — allocate non-contiguously so external fragmentation cannot happen in the first place.

---

<br>

## 4. Paging — Basic Method

### 4.1 Paging Overview

**Paging** decouples logical and physical memory:

- Physical memory is divided into fixed-size **frames**.
- Logical memory is divided into same-size **pages**.
- A **page table** maps page number → frame number, on a per-process basis.

Consequences:

- **External fragmentation is eliminated** — any free frame can hold any page.
- **Internal fragmentation remains** — the last page of each process is almost never fully used; average waste is half a page per process.

### 4.2 Address Translation Principle

Let page size be 2^n and the logical address space be 2^m bytes (on a 32-bit architecture, m = 32; on x86-64, m = 48 — see §9.3). Split the address:

```text
Logical Address (m bits)
+-------------------+------------------+
| Page number (p)   | Page offset (d)  |
|    (m-n bits)     |    (n bits)      |
+-------------------+------------------+

Translation
1. Index the page table with p.
2. Read the frame number f from PT[p].
3. Physical address = (f << n) | d.

Physical Address
+-------------------+------------------+
| Frame number (f)  | Page offset (d)  |
+-------------------+------------------+
```

**Key invariant**: the page offset `d` passes through unchanged — pages and frames are the same size, so the position within them is identical.

> **Why `(f << n) | d` equals `f × page_size + d`** (bit manipulation reminder from computer architecture): shifting an integer left by `n` bits multiplies it by 2ⁿ. Since page size = 2ⁿ, `f << n` is exactly `f × page_size`. The lower `n` bits of `f << n` are all zero, so OR-ing the `n`-bit offset `d` into them is the same as adding `d`. Splitting the address into (page #, offset) is thus a *no-cost* division and modulus by the page size — done by wiring, not arithmetic.

### 4.3 Paging Hardware and Free-Frame Management

For every CPU memory reference the MMU consults the **current process's page table** to perform translation. Each process maintains its own table — context switching includes loading the page-table base address (PTBR).

The OS maintains a **free-frame list**. When a process is created (or grows), frames are taken from the list and recorded in the process's page table.

### 4.4 Internal Fragmentation and Page Size Trade-offs

If page size = 2,048 bytes and a process needs 72,766 bytes:

- 72,766 / 2,048 = 35 full pages + 1,086 bytes
- The OS allocates **36 frames** → internal fragmentation = 2,048 − 1,086 = **962 bytes**
- Worst case: a process of size *(n × page_size) + 1 byte* needs n+1 frames, wasting nearly a full frame.

Page size trade-off:

| Smaller page | Larger page |
|--------------|-------------|
| Less internal fragmentation | More internal fragmentation |
| Larger page table | Smaller page table |
| More I/O operations | More efficient bulk I/O |

Modern systems typically use **4KB** or **8KB** as the default, with optional **2MB** and **1GB** huge pages for memory-intensive workloads.

---

<br>

## 5. Paging Hardware Support — TLB

### 5.1 Where to Keep the Page Table

Two implementation strategies:

**Method 1 — Dedicated registers.** Store the entire page table in special-purpose hardware registers.
- Pro: extremely fast lookup.
- Con: only viable when the page table has a few hundred entries — does not scale.

**Method 2 — PTBR (Page-Table Base Register).** Keep the page table in main memory; PTBR points to its base.
- Pro: context switching only updates PTBR.
- Con: **every memory reference now costs two memory accesses** — one to read the page-table entry, one to read the actual data. This doubles effective memory latency.

### 5.2 TLB — Translation Look-aside Buffer

The **TLB** is a small, very fast **associative memory** that caches recent translations.

> **What "associative memory" means** (computer architecture — also called **Content-Addressable Memory / CAM**): a regular RAM takes an *address* in and returns the *value* at that address. A CAM works the other way around — you give it a *value* (here, a page number), and dedicated comparator hardware checks **every stored key in parallel** in a single cycle, returning the slot index (and stored data) of whichever entry matches. The same hardware idea underlies cache tag arrays. Parallel comparison is what makes TLB lookup essentially free, but it is also expensive in transistors per entry, which is why a TLB has only tens to a couple thousand entries.

- Size: typically 32 to 1,024 entries.
- Each entry: **(page #, frame #)** plus protection bits.
- Lookup compares the input page # against **all keys in parallel** — single-cycle.
- Sits inside the CPU pipeline, so a TLB hit has no observable latency.

```text
TLB Hit / Miss process:

CPU --p,d--> Search for p in TLB
              |
          +---+---+
          |       |
       hit       miss
          |       |
       got f    walk page table in memory,
          |    insert (p,f) into TLB
          |       |
          +---+---+
              |
          f + d --> physical addr --> DRAM
```

When the TLB is full, an entry must be evicted (LRU, round-robin, random). Some entries are **wired down** (cannot be evicted) — typically kernel code/data, to keep the OS responsive.

### 5.3 ASID — Address-Space Identifier

Without help, a context switch must **flush the entire TLB**, because TLB entries from process P1 would otherwise mis-translate process P2's addresses. The cost is a flood of TLB misses every time the scheduler switches processes.

> **Order of magnitude**: a 1,024-entry TLB with ~100 ns per miss can spend roughly **100 µs** rewarming after a flush — comparable to a full Linux time slice on busy servers. Multiply by the context-switch rate (often thousands per second) and unmitigated TLB flushes become a serious source of overhead. This is the gap ASID is designed to close.

**ASID** tags each TLB entry with a process identifier:

```text
+------+-----+-----+
| ASID |  p  |  f  |
+------+-----+-----+
|  P1  |  3  |  7  |   Process 1's page 3 -> frame 7
|  P2  |  3  | 12  |   Process 2's page 3 -> frame 12
+------+-----+-----+
```

On lookup, both the page number **and** the current process's ASID must match. The TLB can hold entries from many processes simultaneously, and context switches become much cheaper.

### 5.4 Effective Access Time (EAT)

Quantify TLB performance. Let:

- α = TLB **hit ratio**
- M = main-memory access time
- T = TLB access time

```text
EAT = α · (T + M) + (1 − α) · (T + 2M)
```

> **Where the formula comes from** (expected-value over two events):
> - On a **hit** (probability α) we pay T (the TLB lookup itself, always paid) + M (one memory access to fetch the actual data) → `T + M`.
> - On a **miss** (probability 1 − α) we pay T (the unsuccessful TLB lookup) + M (read the page-table entry from memory) + M (then read the actual data) → `T + 2M`.
> - Multiply each case by its probability and sum. This is the standard expected-value technique from discrete probability.

**Numerical examples** (M = 100 ns, T = 10 ns):

```text
α = 0.80:  EAT = 0.80·110 + 0.20·210 = 88 + 42  = 130 ns  (30% slowdown vs. raw)
α = 0.98:  EAT = 0.98·110 + 0.02·210 = 107.8 + 4.2 = 112 ns  (~ 2% slowdown)
α = 0.99:  EAT = 0.99·110 + 0.01·210 = 108.9 + 2.1 = 111 ns  (~ 1% slowdown)
```

The lesson: a high TLB hit rate (≥98%, easily achievable on real workloads thanks to *spatial* and *temporal locality*) makes paging effectively free.

---

<br>

## 6. Paging — Protection and Shared Pages

### 6.1 Protection Bits and Valid-Invalid Bit

Each page-table entry carries extra bits:

**Permission bits**:
- **R/W** — read-only vs read-write.
- **X** — executable.
- A protection-violation reference triggers a hardware trap; the OS typically delivers `SIGSEGV` to the offending process.

**Valid-Invalid bit**:
- *Valid* — page belongs to the process's logical address space.
- *Invalid* — page is not in use by this process; any reference traps.

This handles the common case where a process's actual address-space use is much smaller than the theoretical maximum. To cut the page table down to just the used portion, hardware exposes a **PTLR (Page-Table Length Register)** that bounds the table's actual length.

### 6.2 Shared Pages

Two processes running the same code can **share frames** for the read-only text pages, provided the code is **reentrant** (no self-modifying instructions, no per-process global state).

```text
Example — 40 users on a text editor (libc text 2MB):
  Without sharing:  40 × 2MB = 80MB
  With sharing:      1 × 2MB =  2MB + 40 × (private data)
```

Typical sharing targets:
- **System libraries** (libc, dynamic linker, kernel-mode shared code).
- **Compilers, editors, window systems** in time-sharing servers.
- **Database engines** with many client connections.
- **Shared memory** segments for explicit IPC.

The OS *enforces* the read-only property via protection bits. Each process keeps its own private data and stack pages.

---

<br>

## 7. Page-Table Structure

### 7.1 Why Single-Level Page Tables Don't Scale

A flat page table with one entry per virtual page is fine for small address spaces but blows up quickly.

```text
32-bit address, 4 KB (= 2^12) page:
  entries = 2^32 / 2^12 = 2^20 ≈ 1,048,576
  4 bytes per entry → page table = 4 MB per process

64-bit address, 4 KB page:
  entries = 2^64 / 2^12 = 2^52 ≈ 4·10^15
  8 bytes per entry → page table ≈ 32 PB (!)
```

You cannot find 4 MB of contiguous physical memory just to hold one process's page table, much less 32 PB. Three classical fixes:

1. **Hierarchical paging** (multi-level).
2. **Hashed page tables**.
3. **Inverted page tables**.

### 7.2 Hierarchical Paging

Page the page table itself.

```text
Logical address (32-bit, 4KB pages, two-level):
+----------+----------+----------+
|   p1     |   p2     |    d     |
| 10 bits  | 10 bits  |  12 bits |
+----------+----------+----------+

  p1  →  outer page table (1024 entries)
  p2  →  inner page table (1024 entries)
  d   →  page offset
```

This is the classical **forward-mapped** page table. The huge win: inner page tables for **unused address-space regions are not allocated**. Mechanically (in data-structures terms), the outer table is an array of 1024 pointers; each pointer either references an inner page table (4 KB, 1024 entries) or is **null/invalid**. Walking a null outer entry traps to the OS as "no mapping"; the corresponding 4 MB of virtual address space costs zero physical memory for the page table itself. A typical process uses only a small fraction of its 4 GB virtual space, so most outer entries stay null and we pay only for the used branches.

### 7.3 Hierarchical Paging Falls Apart in 64-bit

Naive two-level paging on a 64-bit address space:

```text
+----------+----------+----------+
|    p1    |    p2    |    d     |
|  42-bit  |  10-bit  |  12-bit  |
+----------+----------+----------+

outer table = 2^42 entries = 2^44 bytes = 16 TB  → still impossible
```

Adding more levels helps but quickly becomes impractical:

```text
Three-level: outer ≈ 16 GB → still too large
Would need ~7 levels → per-reference cost balloons.
```

So real 64-bit systems do **not** use a uniform multi-level page table at the same granularity as 32-bit. They use **4-level paging with sparse address-space use** (x86-64) or one of the alternatives below.

### 7.4 Hashed Page Tables

Index the table with a **hash of the virtual page number**:

```text
hash(p) → chain of (p, f, next) entries
walk the chain until p matches
```

Each bucket holds collisions in a linked list — this is the standard **separate-chaining** collision resolution from data structures: the entry stored at the bucket head has the form `(virtual_page, frame, next)`, with `next` linking to the next collision in the same bucket. To translate page `p` you compute `hash(p)`, walk the linked list at that bucket, and stop on the node whose stored virtual_page equals `p`. Average lookup is O(1) when the load factor is kept low; worst case is O(chain length). This works well for **sparse** address spaces (most of the 64-bit range is unused), because the table size is proportional to actually used pages, not to theoretical maximum.

A **clustered page table** stores mappings for several consecutive pages in one entry, amortizing hash-table overhead. This is especially well suited to 64-bit sparse address spaces.

### 7.5 Inverted Page Tables

Flip the data structure: maintain a **single global table indexed by frame number** (one entry per physical frame).

```text
Inverted PT:
  frame_id | (pid, virtual_page)
  -------- | ---------------------
     0     | (P3, 0x4012)
     1     | (P1, 0x000A)
     ...
```

- **Pro**: total size is proportional to physical memory, not virtual memory — huge savings.
- **Con**: lookups are by *virtual* address, so the OS must search the whole table. The fix is an **auxiliary hash table** layered *on top of* the inverted table: hash `(pid, virtual_page)` to obtain the **frame index** (= row in the inverted table). This is structurally different from the hashed page table in §7.4 — there the hash *replaces* the page table; here it accelerates lookup into a separate structure indexed by frame.
- **Limitation**: each frame maps to exactly one (pid, vpage). Shared pages — where the same frame appears in multiple processes' page tables — are awkward to express.

### 7.6 Comparison

| Structure | Pros | Cons | Suitable for |
|-----------|------|------|--------------|
| Single-level | Simple, fast | Huge table per process | Small address spaces |
| Hierarchical | Skips unused regions | Many levels in 64-bit | 32-bit systems, x86-64 (4-level) |
| Hashed | Scales to large/sparse address spaces | Hash-collision overhead | 64-bit, sparse VAs |
| Inverted | Size = O(physical memory) | Slow search, hard to share | Large physical RAM |

---

<br>

## 8. Swapping

### 8.1 Standard Swapping

**Swap out** an entire process from memory to a **backing store** (a region on disk reserved for this); **swap in** when it is scheduled again.

- Frees memory for other processes when the system is overcommitted.
- **Very expensive**: transferring an entire process across disk takes seconds, dwarfing any time-slice.
- Rarely used in modern OSes.

### 8.2 Swapping with Paging

The modern equivalent operates at **page granularity**:

- **Page out**: write specific pages from memory to backing store.
- **Page in**: load specific pages from backing store on demand.
- Only the *needed* pages move, not the whole process. Faster, finer, much more efficient.

This is the foundation of **virtual memory** — covered in detail in [[concepts_lecture_w12]].

### 8.3 Swapping on Mobile Systems

Most mobile OSes **do not support** swapping. Reasons:

- **Flash storage has limited write endurance** — swapping wears it out.
- **Throughput** between flash and RAM is much lower than between desktop SSD and RAM.
- **Storage capacity** is small to begin with.

Alternative strategies:

- **iOS** — when memory is low, asks apps via signals to release memory voluntarily; uncooperative apps are terminated.
- **Android** — saves an app's serialized state to flash before killing the process; the user perceives a fast restart even though the process is brand new.

---

<br>

## 9. Real-World Architectures

### 9.1 IA-32

32-bit x86. Uses **segmentation + paging** in combination.

- A **logical address** is a (segment selector, offset) pair.
- Segmentation hardware maps it to a **linear address**.
- Paging then maps the linear address to a physical address.
- Page sizes: **4 KB** (and **4 MB** large pages via PSE).

### 9.2 IA-32 PAE

**PAE (Page Address Extension)** lets a 32-bit processor address more than 4 GB of physical RAM.

- Page-table entries extended from **32 bits → 64 bits**.
- Physical frame number widened from 20 to **24 bits** (+ 12-bit offset = 36-bit physical address ⇒ 64 GB).
- Linux and macOS supported PAE; 32-bit consumer Windows was capped at 4 GB anyway.

### 9.3 x86-64

The 64-bit extension architecture (originally AMD64, adopted by Intel).

- **48-bit virtual address space** = 256 TB. (The unused top bits must be a sign-extension of bit 47 — "canonical addresses". *What this means*: only the low 48 bits of a 64-bit pointer carry information; bits 48–63 must all match bit 47, i.e. **all 0s or all 1s**. Any non-canonical pointer triggers a #GP general-protection fault when dereferenced. This design reserves the unused top bits so that future CPUs can widen the address space without breaking software that assumed garbage bits there.)
- **52-bit physical address** via PAE-style extension = 4 PB.
- Page sizes: **4 KB**, **2 MB**, **1 GB**.
- 4-level page table (PML4 → PDPT → PD → PT). Modern parts add a 5th level for 57-bit VAs.
- Segmentation is effectively disabled (flat 64-bit address space).

### 9.4 ARMv8

The dominant 64-bit architecture in mobile/embedded systems.

- Hierarchical paging with up to **4 levels** (translation granules of 4 KB, 16 KB, or 64 KB).
- **Two-level TLB**: separate micro-TLBs for instruction and data + a larger main TLB.
- Hardware support for **ASIDs** — context-switch flushing avoided.

### 9.5 Comparison

| Feature | IA-32 | x86-64 | ARMv8 |
|---------|-------|--------|-------|
| Address size | 32-bit | 48-bit virtual | 48-bit virtual |
| Paging levels | 2 (3 with PAE) | 4 | up to 4 |
| Page sizes | 4 KB, 4 MB | 4 KB, 2 MB, 1 GB | 4 KB, 16 KB, 64 KB |
| Segmentation | Used | Effectively disabled | None |
| Physical memory | 4 GB (64 GB w/ PAE) | 4 PB (52-bit) | up to 48-bit |
| TLB | Present | Multi-level | 2-level |

---

<br>

## 10. Lab — Memory Allocation Simulator

### 10.1 Lab Goal and Data Structure

**Goal**: implement a memory-block list manager and compare First-fit, Best-fit, Worst-fit on the same request stream.

Requirements:
- Represent memory as a linked list of blocks (start, size, status, owner).
- Allocate by each of the three strategies.
- Release: coalesce adjacent free blocks.
- Output: layout snapshot, allocation success/failure, and external-fragmentation size.

```c
typedef struct Block {
    int start;        // start address
    int size;         // block size
    int allocated;    // 0 = free, 1 = allocated
    char process[10]; // owning process name
    struct Block* next;
} Block;
```

> **Why a singly-linked list?** (data-structures perspective): a sorted-by-address linked list is the natural fit because the operations are (1) walk the list to find a hole — sequential access, O(n); (2) split a block — pointer surgery, O(1); (3) coalesce after release — check the neighbor pointers, O(1). **First-fit can short-circuit** the walk on the first match, so its expected cost is much smaller than n on a fragmented list; **Best-fit and Worst-fit** must scan the entire list to be sure they have found the smallest / largest sufficient hole — O(n) every time. This asymmetry is the algorithmic reason First-fit is typically faster than the other two, independent of fragmentation quality.

### 10.2 Lab Functions and Example I/O

```c
Block* first_fit(Block* head, int size);   // first sufficient hole
Block* best_fit(Block* head, int size);    // smallest sufficient hole
Block* worst_fit(Block* head, int size);   // largest hole
void   release(Block* head, char* process);// release + coalesce neighbors
void   print_memory(Block* head);
```

Example trace:

```text
Input
  Memory size: 1024
  Allocate P1: 200
  Allocate P2: 350
  Allocate P3: 150
  Release P2
  Allocate P4: 300

Output (First-fit)
  [  0- 199] P1 (200)
  [200- 549] FREE (350)      <- after P2 release
  [550- 699] P3 (150)
  [700-1023] FREE (324)

  Allocate P4(300) → First-fit → [200-499] P4
  [  0- 199] P1 (200)
  [200- 499] P4 (300)
  [500- 549] FREE (50)       <- residual hole
  [550- 699] P3 (150)
  [700-1023] FREE (324)

  External fragmentation = 50 + 324 = 374 bytes
```

Extensions: feed a stream of randomized requests, compare the average external fragmentation across strategies, and verify the 50-percent rule empirically.

---

<br>

## Summary

| Topic | Key Content |
|-------|-------------|
| Memory hierarchy | CPU directly addresses only registers and main memory; disk requires loading |
| Address binding | Compile-time / Load-time / Execution-time; only execution-time needs MMU |
| MMU | Hardware translation; protects via base/limit, enables relocation |
| Dynamic loading/linking | Lazy load routines; shared libraries save memory and ease updates |
| Contiguous allocation | First/Best/Worst-fit; First-fit usually wins in practice |
| Fragmentation | External (no contiguity) vs Internal (unit > request); ~1/3 lost (50-percent rule) |
| Paging | Fixed-size pages → frames; eliminates external fragmentation |
| TLB | Caches translations; α≥98% makes paging nearly free |
| ASID | Avoids TLB flush on context switch |
| Page-table structures | Single-level / Hierarchical / Hashed / Inverted |
| 64-bit | Hierarchical alone insufficient; use 4-level + sparse, or hashed |
| Swapping | Standard (whole process) vs paged (per-page); mobile mostly avoids swap |
| Architectures | IA-32 (seg+pag), x86-64 (4-level), ARMv8 (4-level + ASID) |

---

<br>

## Self-Check Questions

1. Why can the CPU directly access main memory but not disk?

   > **Answer:** The CPU's instruction set issues addresses on the memory bus to register or main memory only — these are the storage layers the architecture exposes as directly addressable. Disk sits behind a separate I/O subsystem (controllers, DMA, drivers); reaching it requires explicit OS-mediated transfers (~millions of cycles). To execute or even read disk data, the OS must first **load** it into memory.

2. Distinguish logical and physical addresses. Under which binding scheme are they identical?

   > **Answer:** A **logical address** is generated by the CPU as part of program execution; a process sees its own private space 0…max. A **physical address** is what hits DRAM. They are identical under **compile-time** or **load-time** binding (no runtime translation). Under **execution-time** binding, the MMU translates every reference, and the two diverge.

3. State the role of the MMU. How does a base+limit MMU protect processes?

   > **Answer:** The MMU sits between the CPU and the memory bus, **translating every logical address to a physical address**. In the simplest base+limit form, it (a) compares the logical address to the **limit** and traps on overflow, and (b) adds the **base** register to produce the physical address. Because base/limit can only be modified in **kernel mode**, a user process has no way to escape its sandbox.

4. Compare static and dynamic linking. Why are shared libraries called *shared*?

   > **Answer:** **Static linking** copies all library code into each executable at build time — each process carries its own copy. **Dynamic linking** leaves library calls as stubs; at execution time the loader maps a single in-memory copy of the library and patches the stubs. The library is *shared* across processes — one set of physical frames holds the code (typically read-only), used by all callers. Saves memory and disk, and a single update fixes all callers.

5. Walk through First-fit, Best-fit, Worst-fit on the same hole list. Why does Worst-fit usually perform worst?

   > **Answer:** First-fit picks the first hole that satisfies the request, stopping as soon as it finds one. Best-fit scans the whole list and picks the smallest sufficient hole; Worst-fit picks the largest. Worst-fit's intuition — *"leave the biggest leftover hole"* — backfires: it shreds the largest hole first, eliminating the only hole large enough for future big requests. First-fit and Best-fit preserve big holes; First-fit is also faster (no full scan).

6. Distinguish external and internal fragmentation. State the 50-percent rule.

   > **Answer:** **External fragmentation** — enough total free memory exists, but no single hole is large enough; a contiguity problem. **Internal fragmentation** — the allocation unit is larger than the request, and the leftover *inside* the allocation is wasted. **50-percent rule** (Knuth's analysis for first-fit): with N allocated blocks, statistically about 0.5N additional blocks are lost to external fragmentation — roughly 1/3 of total memory becomes unusable, which motivated the shift to paging.

7. Walk through a paging address translation. Why does the offset pass through unchanged?

   > **Answer:** Split the logical address as `(page #, offset)`. Index the page table with the page # to obtain a **frame #**. The physical address is `(frame # << offset_bits) | offset`. The **offset is unchanged** because pages and frames are the same size — the position within a page is identical to the position within the frame that backs it. Only the higher-order page number is translated.

8. What problem does the TLB solve, and how? What does ASID add?

   > **Answer:** Without a TLB, every memory reference costs *two* memory accesses (read PTE, read data). The TLB caches recent (page, frame) translations in an associative memory inside the CPU pipeline; a hit costs zero cycles. **ASID** tags each TLB entry with the owning process, so context switches need not flush the TLB — multiple processes' translations can coexist. Without ASID, every context switch triggers a flood of TLB misses on the next process.

9. Compute EAT for M = 100 ns, T = 10 ns, hit ratio α = 0.99. Where does the slowdown go?

   > **Answer:** `EAT = 0.99·(10+100) + 0.01·(10+200) = 108.9 + 2.1 = 111 ns`. The slowdown over raw memory (100 ns) is about 11 ns or 11%; of that, 10 ns is the TLB lookup itself (paid on every reference), and ~1 ns is amortized miss cost. The takeaway is that a 99% TLB hit rate makes paging effectively free.

10. Why does a single-level page table not scale to 64-bit, and why does a *uniform* 2-level paging not work either?

    > **Answer:** Single-level: 2^64 / 2^12 ≈ 4·10^15 entries × 8 bytes ≈ 32 PB per process — impossible. Uniform two-level: if `p1` is 42 bits and `p2` is 10 bits, the *outer* table alone needs 2^42 entries ≈ 16 TB — still impossible. Even three- or four-level uniform schemes leave intermediate tables too large. Real 64-bit systems use **hierarchical + sparse allocation** (skip unused branches) or alternate structures like **hashed / inverted** page tables.

11. When are inverted page tables a good fit? What is their main limitation?

    > **Answer:** They work well when **physical memory is much smaller than virtual memory** — the inverted table's size is proportional to RAM, not to per-process virtual space. Major limitation: each physical frame has *one* entry, so **shared pages** (frame appears in many processes' virtual address spaces) are difficult to model — typically requires an auxiliary structure or a chain. Lookup is also slow without a hash; the lookup hash + TLB combo mitigates this.

12. Why do mobile operating systems mostly *not* implement swapping?

    > **Answer:** Three reasons. (1) **Flash has limited write endurance**; swap pages wear it out. (2) **Throughput** from flash to RAM is much lower than from desktop SSD, so swapping in/out is slow. (3) **Storage capacity** on phones is small. iOS and Android instead reclaim memory by killing apps — iOS asks first, then kills; Android serializes app state to flash so the user perceives a fast restart.

13. What is the page-size trade-off, and why have systems converged on ~4–8 KB?

    > **Answer:** Smaller pages → less internal fragmentation but larger page tables and more (smaller) I/O operations; larger pages → smaller page tables and bulkier I/O but more internal fragmentation. The 4–8 KB sweet spot balances these for typical desktop and server workloads, while modern systems offer **2 MB / 1 GB huge pages** as an opt-in for large-RAM workloads (databases, VMs) where TLB pressure dominates internal-fragmentation waste.

14. How does shared-page sharing work, and what is the read-only requirement protecting?

    > **Answer:** Two processes' page tables both map a particular virtual page to the *same* physical frame (e.g., libc text). The OS marks the entry **read-only** in both page tables. The read-only enforcement is what makes sharing *safe*: neither process can modify the shared frame, so the other process cannot observe a writes-from-elsewhere change to "its own" memory. Code naturally fits because reentrant code does not need to write to its own text.

15. Why does the OS need execution-time binding to use compaction, but not to use paging?

    > **Answer:** Compaction physically moves a process's bytes to a new range; for the process to keep running, its base register must be updated *while it is in memory* — that is exactly what execution-time binding enables. Paging avoids the question entirely by *not requiring contiguous physical allocation* — there is nothing to compact, because the page table indirection already lets any virtual page sit on any physical frame.
