# Week 7 Concepts Lecture — CPU Scheduling (2)

> **Last Updated:** 2026-04-15
>
> Silberschatz, Operating System Concepts Ch 5 (Sections 5.3.4 – 5.8)

> **Prerequisites**: Week 6 CPU Scheduling (1). You should already understand FCFS, SJF, SRTF, RR, and the five scheduling criteria (CPU utilization, throughput, turnaround, waiting, response time).
>
> **Learning Objectives**:
> 1. Understand Priority Scheduling and the Starvation / Aging mechanism
> 2. Compare Multilevel Queue vs Multilevel Feedback Queue (MLFQ)
> 3. Distinguish PCS (Process-Contention Scope) and SCS (System-Contention Scope) in thread scheduling
> 4. Explain SMP, Processor Affinity, NUMA, Load Balancing, and HMP (big.LITTLE)
> 5. Understand Real-Time scheduling: Rate-Monotonic, EDF, Proportional Share
> 6. Describe Linux CFS (vruntime + Red-Black Tree) and Windows priority-based scheduling
> 7. List the four algorithm evaluation methods (Deterministic, Queueing, Simulation, Implementation)

---

## Table of Contents

- [1. Priority Scheduling](#1-priority-scheduling)
  - [1.1 Priority Scheduling — Concept](#11-priority-scheduling--concept)
  - [1.2 Priority Scheduling — Example](#12-priority-scheduling--example)
  - [1.3 Priority + Round-Robin (Tie-Breaking)](#13-priority--round-robin-tie-breaking)
  - [1.4 Starvation and Aging](#14-starvation-and-aging)
- [2. Multilevel Queue and MLFQ](#2-multilevel-queue-and-mlfq)
  - [2.1 Multilevel Queue Scheduling](#21-multilevel-queue-scheduling)
  - [2.2 Multilevel Feedback Queue (MLFQ)](#22-multilevel-feedback-queue-mlfq)
  - [2.3 MLFQ Design Parameters](#23-mlfq-design-parameters)
  - [2.4 MLFQ Operation Example](#24-mlfq-operation-example)
- [3. Thread Scheduling](#3-thread-scheduling)
  - [3.1 Contention Scope — PCS vs SCS](#31-contention-scope--pcs-vs-scs)
  - [3.2 PCS vs SCS — Comparison](#32-pcs-vs-scs--comparison)
  - [3.3 Pthread Scheduling API](#33-pthread-scheduling-api)
- [4. Multi-Processor Scheduling](#4-multi-processor-scheduling)
  - [4.1 Multiprocessor Architectures](#41-multiprocessor-architectures)
  - [4.2 SMP — Ready Queue Organization](#42-smp--ready-queue-organization)
  - [4.3 Multicore and Memory Stall](#43-multicore-and-memory-stall)
  - [4.4 Chip Multithreading (CMT)](#44-chip-multithreading-cmt)
  - [4.5 Coarse-grained vs Fine-grained Multithreading](#45-coarse-grained-vs-fine-grained-multithreading)
  - [4.6 Two Levels of Scheduling](#46-two-levels-of-scheduling)
  - [4.7 Processor Affinity](#47-processor-affinity)
  - [4.8 NUMA and Processor Affinity](#48-numa-and-processor-affinity)
  - [4.9 Load Balancing](#49-load-balancing)
  - [4.10 Heterogeneous Multiprocessing (HMP)](#410-heterogeneous-multiprocessing-hmp)
- [5. Real-Time CPU Scheduling](#5-real-time-cpu-scheduling)
  - [5.1 Soft vs Hard Real-Time](#51-soft-vs-hard-real-time)
  - [5.2 Event Latency](#52-event-latency)
  - [5.3 Periodic Task Model](#53-periodic-task-model)
  - [5.4 Rate-Monotonic Scheduling](#54-rate-monotonic-scheduling)
  - [5.5 Rate-Monotonic — Success Example](#55-rate-monotonic--success-example)
  - [5.6 Rate-Monotonic — Failure Example](#56-rate-monotonic--failure-example)
  - [5.7 Rate-Monotonic CPU Utilization Bound](#57-rate-monotonic-cpu-utilization-bound)
  - [5.8 Earliest-Deadline-First (EDF)](#58-earliest-deadline-first-edf)
  - [5.9 EDF — Solving the Rate-Monotonic Failure Case](#59-edf--solving-the-rate-monotonic-failure-case)
  - [5.10 Rate-Monotonic vs EDF](#510-rate-monotonic-vs-edf)
  - [5.11 Proportional Share Scheduling](#511-proportional-share-scheduling)
- [6. Operating System Examples](#6-operating-system-examples)
  - [6.1 Linux CFS — Overview and History](#61-linux-cfs--overview-and-history)
  - [6.2 Linux Priority Ranges](#62-linux-priority-ranges)
  - [6.3 CFS — vruntime Concept](#63-cfs--vruntime-concept)
  - [6.4 CFS — vruntime Step-by-Step Example](#64-cfs--vruntime-step-by-step-example)
  - [6.5 CFS — Target Latency and Time Slice](#65-cfs--target-latency-and-time-slice)
  - [6.6 CFS — Red-Black Tree](#66-cfs--red-black-tree)
  - [6.7 CFS — Load Balancing and NUMA](#67-cfs--load-balancing-and-numa)
  - [6.8 Windows Scheduling](#68-windows-scheduling)
  - [6.9 Windows — Priority Classes](#69-windows--priority-classes)
  - [6.10 Windows — Variable Priority Behavior](#610-windows--variable-priority-behavior)
  - [6.11 Windows — Multiprocessor Support](#611-windows--multiprocessor-support)
- [7. Algorithm Evaluation](#7-algorithm-evaluation)
  - [7.1 Four Evaluation Methods — Overview](#71-four-evaluation-methods--overview)
  - [7.2 Deterministic Modeling](#72-deterministic-modeling)
  - [7.3 Queueing Models — Little's Formula](#73-queueing-models--littles-formula)
  - [7.4 Simulation and Implementation](#74-simulation-and-implementation)
- [8. Lab — MLFQ Simulator](#8-lab--mlfq-simulator)
  - [8.1 Lab: Queue Configuration and Rules](#81-lab-queue-configuration-and-rules)
  - [8.2 Lab: Data Structure and Skeleton](#82-lab-data-structure-and-skeleton)
  - [8.3 Lab: Core Execution Logic](#83-lab-core-execution-logic)
  - [8.4 Lab: Sample Output](#84-lab-sample-output)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. Priority Scheduling

### 1.1 Priority Scheduling — Concept

Each process is assigned a **priority (integer)**, and the CPU is allocated to the process with the **highest priority** first.

- Generally, a **smaller number means higher priority** (per the Silberschatz textbook convention)
  - Note: Linux reverses this convention for real-time tasks (larger = higher), so always check which convention is being used.
- Both **Preemptive** and **Nonpreemptive** variants exist:
  - **Preemptive**: If a higher-priority process arrives while a lower-priority one is running, the current process is **forcibly preempted**.
  - **Nonpreemptive**: Waits until the current process voluntarily releases the CPU, then picks the highest-priority ready process.
- **SJF is a special case of Priority Scheduling** — priority = inverse of the predicted next CPU burst (shortest burst = highest priority).

> **Key Insight**: Priority scheduling is a *framework*, not a single algorithm. You choose how priorities are assigned (external: user/admin-specified, or internal: computed from burst time, memory usage, etc.). SJF, SRTF, and many real-time algorithms are all priority schemes with different priority definitions.

### 1.2 Priority Scheduling — Example

All processes arrive at time 0; **smaller number means higher priority**.

| Process | Burst Time | Priority |
|---------|-----------|----------|
| P1 | 10 | 3 |
| P2 | 1  | 1 |
| P3 | 2  | 4 |
| P4 | 1  | 5 |
| P5 | 5  | 2 |

Execution order (by priority): **P2 (1) → P5 (2) → P1 (3) → P3 (4) → P4 (5)**

```text
Gantt Chart:
+----+--------+------------------+------+----+
| P2 |   P5   |        P1        |  P3  | P4 |
+----+--------+------------------+------+----+
0    1        6                 16      18   19
```

| Process | Waiting Time |
|---------|--------------|
| P1 | 6 |
| P2 | 0 |
| P3 | 16 |
| P4 | 18 |
| P5 | 1 |
| **Average** | **(6+0+16+18+1)/5 = 8.2 ms** |

### 1.3 Priority + Round-Robin (Tie-Breaking)

Processes with the **same priority** are scheduled using **Round-Robin (RR)**, so no single process monopolizes its priority level.

| Process | Burst | Priority |
|---------|-------|----------|
| P1 | 4 | 3 |
| P2 | 5 | 2 |
| P3 | 8 | 2 |
| P4 | 7 | 1 |
| P5 | 3 | 3 |

Time quantum = 2 ms; smaller number = higher priority.

```text
Gantt Chart:
| P4          | P2 | P3 | P2 | P3 | P2 | P3     | P1 | P5 | P1 | P5 |
0             7    9   11   13   15   16   20     22   24   26  27
```

Execution flow:
- **Priority 1 (P4)** runs first to completion (nothing else at priority 1).
- **Priority 2 (P2, P3)** alternate via RR (quantum = 2).
- **Priority 3 (P1, P5)** alternate via RR last.

> **Why combine Priority + RR?** Pure priority scheduling makes all processes at the same priority run in arrival order — which means a long process at priority 2 still delays every other priority-2 process. Adding RR ensures **fairness within each priority class**.

### 1.4 Starvation and Aging

**Starvation (Indefinite Blocking)** — a pathology of priority scheduling
- A low-priority process may **never get to execute** because higher-priority processes keep arriving.
- *Historical anecdote*: When MIT shut down an IBM 7094 mainframe in 1973, they discovered a low-priority job that had been submitted in **1967** and had never run. It had waited 6 years.

**Aging — the standard solution**
- **Gradually increase** the priority of a process the longer it waits in the ready queue.
- Example: If priorities range from 0 (highest) to 127 (lowest), increase priority by 1 every second.
  - A brand-new process starting at 127 reaches priority 0 (highest) in about **127 seconds ≈ 2 minutes**.
- Guarantees that every process will **eventually** be selected, no matter how low its initial priority.

> **Exam Tip**: "Starvation" and "Aging" are a matched pair. Whenever you describe a priority-based algorithm, you must address starvation. Aging is the canonical remedy. The same pattern reappears in MLFQ (lower-queue promotion) and CFS (vruntime makes waiting tasks naturally favored).

---

<br>

## 2. Multilevel Queue and MLFQ

### 2.1 Multilevel Queue Scheduling

The ready queue is split into multiple **separate queues**, each for a different class of process.

```text
     Highest priority
     +-----------------------------+
 --> | System processes            |  -->
     +-----------------------------+
     +-----------------------------+
 --> | Interactive processes       |  -->
     +-----------------------------+
     +-----------------------------+
 --> | Interactive editing procs   |  -->
     +-----------------------------+
     +-----------------------------+
 --> | Batch processes             |  -->
     +-----------------------------+
     +-----------------------------+
 --> | Student processes           |  -->
     +-----------------------------+
     Lowest priority
```
*Silberschatz, Figure 5.8 — Multilevel queue scheduling*

**Scheduling between queues** (two common policies):
- **Fixed priority**: A lower queue runs only when **all higher queues are empty**. *(Risk: starvation of lower queues.)*
- **Time-slice allocation**: Each queue gets a fixed percentage of CPU time (e.g., foreground 80%, background 20%).

**Key restriction**: Processes **cannot move between queues** — queue assignment is **fixed** for the lifetime of the process.

> **When to use Multilevel Queue**: When you can classify processes cleanly at creation time (e.g., "this is an interactive user program", "this is a batch job", "this is a daemon"). The rigid separation is a strength when the classification is correct, and a weakness when it isn't — hence MLFQ.

### 2.2 Multilevel Feedback Queue (MLFQ)

The **key difference** from a plain multilevel queue: **processes can move between queues** based on observed behavior.

```text
                      quantum = 8
     +--------+
 --> |   Q0   | -------> CPU
     +--------+   (exhausted)
         |  demote
         v
                      quantum = 16
     +--------+
 --> |   Q1   | -------> CPU
     +--------+   (exhausted)
         |  demote
         v
                      FCFS
     +--------+
 --> |   Q2   | -------> CPU
     +--------+
```
*Silberschatz, Figure 5.9 — Multilevel feedback queues*

**Movement rules:**
- **Demotion**: If a process uses its **entire time quantum** in a given queue, it is demoted to a lower queue. *(Interpretation: it's CPU-bound; demoting it keeps it out of the way of short, responsive processes.)*
- **Promotion (Aging)**: If a process waits too long in a lower queue, it is promoted to a higher queue. *(Prevents starvation.)*

**Natural behavior:**
- **I/O-bound processes** have **short CPU bursts**, so they release the CPU before exhausting their quantum → they **stay in upper (high-priority) queues** → great response time.
- **CPU-bound processes** consume their full quantum → they **sink to lower queues** → they get CPU time eventually, but do not block interactive work.

> **Why MLFQ is powerful**: It automatically classifies processes as I/O-bound vs CPU-bound **without any hints from the programmer**. The scheduler learns from behavior — a process's queue level is effectively a learned estimate of its future CPU-burst length.

### 2.3 MLFQ Design Parameters

To fully define an MLFQ, you must decide:

| Parameter | Description |
|-----------|-------------|
| Number of queues | How many priority levels? (typically 3–5) |
| Algorithm per queue | RR (with what quantum)? FCFS for the bottom queue? |
| Demotion criteria | When does a process drop? (quantum exhaustion is typical) |
| Promotion criteria | When does a process rise? (aging threshold) |
| Initial queue assignment | Where do new processes start? (usually the top queue) |

MLFQ is the **most general and flexible scheduling algorithm** — but it is also the **hardest to tune** because all these parameters interact. Modern schedulers (CFS, Windows variable-priority) are essentially refinements of MLFQ ideas.

### 2.4 MLFQ Operation Example

Suppose process **P** has a CPU burst of 30 ms and enters an MLFQ configured as:
- **Q0**: RR with quantum 8, **Q1**: RR with quantum 16, **Q2**: FCFS (no quantum).

```text
Step 1: P runs 8 ms in Q0 (RR, q=8)
        remaining burst = 22 ms, quantum exhausted -> demoted to Q1

Step 2: P runs 16 ms in Q1 (RR, q=16)
        remaining burst = 6 ms, quantum exhausted -> demoted to Q2

Step 3: P runs 6 ms in Q2 (FCFS)
        remaining burst = 0 -> completed
```

Classification based on total burst length:
- Burst ≤ 8 ms → finishes entirely in **Q0** (top priority, best response).
- 8 < burst ≤ 24 ms → demoted only to **Q1**.
- Burst > 24 ms → sinks all the way to **Q2** (FCFS).

**Net effect**: MLFQ **favors short processes** (they never leave the top queue) while still **ensuring long processes eventually execute** (they finish in Q2).

---

<br>

## 3. Thread Scheduling

### 3.1 Contention Scope — PCS vs SCS

Thread scheduling is categorized by **Contention Scope** — the set of threads competing for a CPU.

**PCS (Process-Contention Scope)**
- **User-level thread** scheduling, performed by the **thread library** (not the kernel).
- Threads **within the same process** compete for a single kernel-scheduled entity.
- Used in **Many-to-One** and **Many-to-Many** threading models.
  - *(Recall from Week 5: Many-to-One maps many user threads onto one kernel thread; Many-to-Many maps N user threads onto M ≤ N kernel threads.)*

**SCS (System-Contention Scope)**
- **Kernel-level thread** scheduling, performed by the **OS kernel**.
- Threads compete with **all other threads in the system** (not just within the same process).
- Used in the **One-to-One** model.
- Modern operating systems (**Linux, Windows, macOS**) all use One-to-One → they effectively only support **SCS**.

> **Why does this matter?** In PCS, if one user thread blocks on a system call, *the entire process's kernel thread blocks*, and the library can't schedule other user threads until it unblocks. In SCS, each thread has its own kernel thread, so one blocking won't block the others. This is why modern OSes prefer One-to-One / SCS despite the higher per-thread cost.

### 3.2 PCS vs SCS — Comparison

| Property | PCS | SCS |
|----------|-----|-----|
| Scheduling entity | Thread library (in user space) | OS kernel |
| Contention scope | Within a single process | System-wide (all threads) |
| Thread model | Many-to-One, Many-to-Many | One-to-One |
| Priority adjustment | Set by programmer; library does not adjust dynamically | Kernel can **dynamically adjust** (e.g., boost on I/O) |
| Time slicing | Not guaranteed among same-priority threads | Guaranteed by kernel timer |

Because **modern OSes use One-to-One**, in practice **only SCS is supported**. PCS is a historical / embedded-systems concept.

### 3.3 Pthread Scheduling API

POSIX threads let you query and set the contention scope per thread **attribute**.

```c
#include <pthread.h>
#include <stdio.h>
#define NUM_THREADS 5

int main(int argc, char *argv[]) {
    int i, scope;
    pthread_t tid[NUM_THREADS];
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    /* Query the current contention scope */
    if (pthread_attr_getscope(&attr, &scope) != 0)
        fprintf(stderr, "Unable to get scheduling scope\n");
    else {
        if (scope == PTHREAD_SCOPE_PROCESS)
            printf("PTHREAD_SCOPE_PROCESS\n");
        else if (scope == PTHREAD_SCOPE_SYSTEM)
            printf("PTHREAD_SCOPE_SYSTEM\n");
    }

    /* Request SCS (system-wide contention) */
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    for (i = 0; i < NUM_THREADS; i++)
        pthread_create(&tid[i], &attr, runner, NULL);
    for (i = 0; i < NUM_THREADS; i++)
        pthread_join(tid[i], NULL);
    pthread_exit(0);
}
```

- `PTHREAD_SCOPE_PROCESS` requests PCS; `PTHREAD_SCOPE_SYSTEM` requests SCS.
- **On Linux and macOS, only `PTHREAD_SCOPE_SYSTEM` is allowed** — requesting `PROCESS` returns an error. This reflects the One-to-One thread model used by these kernels.

---

<br>

## 4. Multi-Processor Scheduling

### 4.1 Multiprocessor Architectures

"Multiprocessor" now encompasses many different hardware configurations:

- **Multicore CPUs** — multiple cores on a single chip (today's norm).
- **Multithreaded cores** — multiple hardware threads per core (e.g., Intel Hyper-Threading).
- **NUMA systems** — Non-Uniform Memory Access (different cores have different memory latencies).
- **Heterogeneous multiprocessing (HMP)** — cores with different performance / power (e.g., ARM big.LITTLE).

Two high-level scheduling approaches:

- **Asymmetric Multiprocessing (ASMP)**: A single **master processor** makes all scheduling decisions; other processors run user code only. *Simple but the master is a bottleneck.*
- **Symmetric Multiprocessing (SMP)**: **Every processor schedules independently** — each one pulls a process from a queue and runs it. *The dominant modern approach (Linux, Windows, macOS).*

### 4.2 SMP — Ready Queue Organization

Under SMP, there are two main ways to organize ready queues:

*Silberschatz, Figure 5.11 — Organization of ready queues*

| Approach | Pros | Cons |
|----------|------|------|
| **Common (shared) ready queue** | Automatic load balancing — no CPU is idle while another has work | **Lock contention** on every access; cache-unfriendly (a process may jump between CPUs) |
| **Per-processor queue** | No locks needed; cache-friendly (a process stays on one CPU) | Possible **load imbalance** — one CPU idle while another is overloaded |

**Modern SMP systems (Linux CFS, Windows) use per-processor queues**, and solve imbalance via **load balancing** (Section 4.9).

### 4.3 Multicore and Memory Stall

**Memory Stall**
- The CPU must wait for data to arrive when it accesses memory that isn't already in cache.
- A cache miss can cost **tens to hundreds of CPU cycles**.
- In memory-intensive workloads, a core can spend **up to 50% of its time stalled** on memory access.

*Silberschatz, Figure 5.12 — Memory stall*
*Silberschatz, Figure 5.13 — Multithreaded multicore system*

**The solution — hardware multithreading**: When one thread enters a memory stall, the core **switches to another hardware thread** that has work ready. This keeps the execution units busy and dramatically improves core utilization.

> **Key idea**: Memory stalls are a *hardware-level* scheduling problem. The core's solution (CMT / SMT, next section) is a miniature scheduler that runs below the OS scheduler's awareness. The OS sees each hardware thread as a full "logical processor" and doesn't need to know about stalls.

### 4.4 Chip Multithreading (CMT)

Multiple **hardware threads** placed on a single physical core.

*Silberschatz, Figure 5.14 — Chip multithreading*

Examples:
- **Intel Hyper-Threading (SMT)** — 2 hardware threads per core.
- **Oracle SPARC M7** — 8 hardware threads per core × 8 cores = **64 logical CPUs**.

Each hardware thread appears to the OS as a **logical processor**. So on a quad-core CPU with Hyper-Threading, Linux reports **8 CPUs** in `/proc/cpuinfo`, even though only 4 are physical.

### 4.5 Coarse-grained vs Fine-grained Multithreading

Two strategies for when the hardware switches between hardware threads:

| Property | Coarse-grained | Fine-grained |
|----------|---------------|-------------|
| Thread switch trigger | Only on **long-latency events** (memory stall, etc.) | At **every instruction cycle boundary** |
| Switch cost | **High** — pipeline must be flushed | **Low** — switch logic is built directly into the pipeline |
| Pros | Simpler hardware design | Very fast switching; high core utilization |
| Cons | Pipeline flush overhead is wasteful | More complex hardware |

> **What is a "pipeline"?** Modern CPUs process instructions in an assembly-line fashion: while instruction 1 is being executed, instruction 2 is being decoded, instruction 3 is being fetched, and so on — typically 5 to 20 stages deep. This "pipeline" is what lets a CPU complete roughly one instruction per cycle despite each instruction actually taking many cycles. A **pipeline flush** means throwing away all the partially-processed instructions in the pipeline (e.g., when switching threads, you can't keep the old thread's in-flight instructions). Every flush wastes the work already done on those stages — hence the high cost of coarse-grained switching.

Modern Intel / SPARC designs use **fine-grained** (or a hybrid) to keep cores busy at instruction-cycle granularity.

### 4.6 Two Levels of Scheduling

On a **multithreaded multicore** processor, scheduling happens at two levels:

*Silberschatz, Figure 5.15 — Two levels of scheduling*

**Level 1 (OS scheduler)**: Assigns **software threads** to **logical processors** (hardware threads). Uses CFS, Windows scheduler, etc.

**Level 2 (Core hardware)**: Decides which hardware thread on a core runs in each cycle. Approaches:
- **Round-robin** (UltraSPARC T3): cycle through all hardware threads fairly.
- **Dynamic urgency** (Intel Itanium): each hardware thread has an urgency value (0–7) that rises during stalls and falls when instructions retire — pick the highest-urgency one.

> **Why two levels**: The OS can't usefully make decisions at cycle granularity — it doesn't have the timing information, and the overhead of a system call per cycle would dwarf the work done. So the OS does coarse-grained placement; the hardware fills in the gaps at cycle granularity.

### 4.7 Processor Affinity

**Affinity** = the tendency to **keep a process on the same processor** once it has started running there.

**Why it matters:**
- Every CPU has its own **cache**. A process running on CPU 0 populates CPU 0's cache with its working set.
- If the OS moves that process to CPU 1, the process's cached data is **invalidated** — it must be re-fetched from memory on CPU 1 (a **warm-up cost**).

| Type | Description |
|------|-------------|
| **Soft Affinity** | OS *tries* to keep a process on the same CPU, but does not guarantee it |
| **Hard Affinity** | The process (or admin) **explicitly specifies** a set of CPUs it may run on — the OS will not migrate it elsewhere |

On Linux, hard affinity is set via the `sched_setaffinity()` system call (or the `taskset` command).

### 4.8 NUMA and Processor Affinity

**NUMA (Non-Uniform Memory Access)** — a multi-socket architecture where each CPU (socket) has its own **local memory bank**, but can also access other sockets' memory **across an interconnect**.

*Silberschatz, Figure 5.16 — NUMA and CPU scheduling*

- **Local memory access**: fast (low latency).
- **Remote memory access**: slow (must traverse the interconnect).

**NUMA-aware scheduling**: place each process on the CPU that is **local to** the memory it allocated. On Linux, CFS considers NUMA via a **scheduling domain hierarchy** (we'll see this in Section 6.7).

> **Practical impact**: Ignoring NUMA on a multi-socket server can cause a 2× performance drop on memory-intensive workloads. This is why high-performance database and HPC code often uses `numactl` or explicit NUMA APIs.

### 4.9 Load Balancing

With per-processor queues, processors can become imbalanced — one CPU has a long run queue while another is idle. **Load balancing** redistributes work.

| Approach | Description |
|----------|-------------|
| **Push migration** | A periodic task checks every CPU's load and **pushes** tasks from overloaded CPUs to idle ones |
| **Pull migration** | An idle CPU actively **pulls** a task from a busy CPU's queue |

Most systems **use both together** (e.g., Linux CFS).

**Load Balancing vs Processor Affinity — a conflict**:
- **Balancing** wants to move processes around to even out load.
- **Affinity** wants to keep processes on the same CPU to preserve cache state.

The OS balances the two: migrate only when the imbalance is **large enough** that the rebalance benefit outweighs the cache-warmup cost.

### 4.10 Heterogeneous Multiprocessing (HMP)

A system with cores that have the **same instruction set** but **different clock speeds and power profiles**.

**ARM big.LITTLE architecture** (used in most modern smartphones):
- **big core**: high performance, high power — great for short interactive tasks (touch response, app launch).
- **LITTLE core**: low performance, low power — great for background tasks (polling, music playback).

**Scheduling strategy:**
- Interactive / foreground apps → **big cores**.
- Background tasks → **LITTLE cores**.
- Battery-saver mode → disable big cores entirely, run only on LITTLE cores.

**Key distinction from ASMP**: Unlike asymmetric multiprocessing, in HMP **all cores can run both user and system tasks** — the asymmetry is in performance/power, not role.

---

<br>

## 5. Real-Time CPU Scheduling

### 5.1 Soft vs Hard Real-Time

| Category | Soft Real-Time | Hard Real-Time |
|----------|----------------|----------------|
| Deadline | Misses **allowed** (performance degrades gracefully) | Deadlines **must be met** — missing one is a system failure |
| Guarantee | Priority-based preferential treatment | Guaranteed completion within deadline |
| Examples | Video playback, VoIP, online games | Aircraft flight control, ABS brakes, pacemakers |

**Real-time scheduling requirements:**
- The scheduler **must be preemptive** (a higher-priority real-time task must be able to preempt anything).
- **Interrupt latency** and **dispatch latency** must be **bounded and small** (microseconds, not milliseconds).

### 5.2 Event Latency

**Event Latency** = total time from event occurrence to service completion.

*Silberschatz, Figure 5.17 — Event latency*

For real-time systems, two components of latency matter:

1. **Interrupt Latency** — from interrupt arrival to the start of the **ISR** (Interrupt Service Routine).
   - Complete the current instruction → determine the interrupt type → save the context.

2. **Dispatch Latency** — time for the scheduler to switch from the currently-running process to the target real-time process.
   - **Conflict phase**: preempt the kernel and release any resources the preempted process holds.
   - **Dispatch phase**: load the high-priority process onto the CPU (context restore, jump to PC).

For hard real-time, both latencies must be **bounded** at a few microseconds.

### 5.3 Periodic Task Model

Real-time workloads are commonly modeled as **periodic tasks** — the same work repeats every *p* units of time.

*Silberschatz, Figure 5.20 — Periodic task*

- **t** — processing time (CPU burst per period).
- **d** — deadline (by when the task's work for this period must be done).
- **p** — period (how often the task repeats).
- **Relationship**: `0 ≤ t ≤ d ≤ p` — you need enough time, within the deadline, within the period.
- **Task rate** = 1/p (how often per second it fires).
- **CPU utilization** of the task = t/p (fraction of the CPU it demands).

> **Mental model**: Think of a video decoder that must decode one frame every 33 ms (for 30 FPS). Period = 33 ms, deadline = 33 ms, burst = however long decoding takes (say 10 ms). If your scheduler misses the deadline even once, the user sees a dropped frame.

### 5.4 Rate-Monotonic Scheduling

**Rate-Monotonic (RM)** — a fixed-priority, preemptive algorithm:

- Assigns **higher priority to tasks with shorter periods**. (The faster the period, the more urgent.)
- Priority = 1 / period.
- Priority is **static** — it never changes during execution.
- Must complete every periodic instance within its deadline.

**Optimality**:
- Rate-Monotonic is **optimal among fixed-priority algorithms**.
- This means: if a set of periodic tasks is not schedulable under RM, then **no fixed-priority algorithm can schedule it**.

### 5.5 Rate-Monotonic — Success Example

- **P1**: period = 50, burst = 20.
- **P2**: period = 100, burst = 35.

**CPU utilization**: 20/50 + 35/100 = 0.40 + 0.35 = **75%**.

Rate-Monotonic priority: **P1 > P2** (shorter period).

Deadlines within one hyperperiod: P1 @ 50, P2 @ 100, P1 @ 100, P1 @ 150, P2 @ 200.

```text
Gantt chart (period 0 - 100):

|<-- P1 (20) -->|<------ P2 (30) ------>|<-- P1 (20) -->|<P2(5)>|<--- idle (25) --->|
0              20                      50              70      75                  100
                                                       ^
                                                   P1's new period starts at 50
                                                   -> P1 preempts P2
```

Step-by-step:
- **0–20**: P1 runs (all 20 ms done). P1's deadline 50 will be met. ✓
- **20–50**: P2 runs for 30 ms of its 35 ms budget.
- **50**: P1's **next period** begins (new deadline at 100). Because RM gives P1 higher fixed priority, P1 **preempts** P2.
- **50–70**: P1 runs (all 20 ms done). P1's deadline 100 will be met. ✓
- **70–75**: P2 resumes for the remaining 5 ms. P2 completes at 75, well before its deadline of 100. ✓
- **75–100**: CPU is idle (no runnable task until P1's next period starts at 100).

**All deadlines met.**

### 5.6 Rate-Monotonic — Failure Example

- **P1**: period = 50, burst = 25.
- **P2**: period = 80, burst = 35.

**CPU utilization**: 25/50 + 35/80 = 0.50 + 0.4375 = **93.75%**.

Theoretical RM upper bound for N=2 tasks: `N(2^(1/N) - 1) = 2(√2 - 1) ≈ 82.8%`.

Since **93.75% > 82.8%**, RM may fail — let's see:

```text
 |<-- P1(25ms) -->|<-- P2(25ms) -->| P1preempts |<P2(10ms)>|
 0               25               50       75        85
                                                    ^
                                          P2's deadline = 80
                                          MISSED!
```

- **P1**: runs 0–25 (done), then 50–75 (done). P1 meets its deadlines.
- **P2**: runs 25–50 (25 ms), then preempted by P1; resumes at 75 and finishes at **85**.
- **But P2's deadline was 80** → **deadline miss!**

Utilization exceeded the RM bound → RM could not guarantee schedulability.

### 5.7 Rate-Monotonic CPU Utilization Bound

The worst-case CPU utilization bound for N tasks under RM:

> **U = N(2^(1/N) − 1)**

| N (number of tasks) | Utilization bound |
|:-------------------:|:-----------------:|
| 1 | 100% |
| 2 | 82.8% |
| 3 | 78.0% |
| 4 | 75.7% |
| 5 | 74.3% |
| N → ∞ | **69.3%** (= ln 2) |

**If total utilization ≤ this bound, RM guarantees all deadlines are met.**

Above the bound, it *may* still succeed (the bound is sufficient, not necessary), but it's not guaranteed.

> **Intuition**: The bound gets tighter as N grows because with many tasks of different periods, preemptions can pile up in unlucky ways. In the limit, you have to "leave 30% of the CPU empty" to absorb these worst-case overlaps.

### 5.8 Earliest-Deadline-First (EDF)

**EDF** — a dynamic-priority, preemptive algorithm:

- At any moment, run the task whose **next deadline is closest**.
- Priority **changes dynamically** as deadlines shift (each time a task completes, a new deadline starts).
- Works for both **periodic and aperiodic** tasks.
- **Theoretically schedulable up to 100% CPU utilization** — EDF is optimal in the strict sense.

**In practice**: context-switch overhead and interrupt handling mean you can't truly hit 100%. But EDF still outperforms RM on utilization, often substantially.

### 5.9 EDF — Solving the Rate-Monotonic Failure Case

Same task set that RM failed on:
- **P1**: period = 50, burst = 25.
- **P2**: period = 80, burst = 35.

EDF trace:

```text
 Time 0:  P1 deadline=50, P2 deadline=80  -> P1 first (deadline closer)
 Time 25: P1 done; P2 starts
 Time 50: P1 new period, P1 deadline=100, P2 deadline=80 (still)
          P2's deadline is closer -> P2 continues! (unlike RM which would preempt)
 Time 60: P2 done (35 ms total executed); P1 starts
 Time 85: P1 done (25 ms executed)
```

```text
 |<-- P1(25) -->|<----- P2(35) ----->|<-- P1(25) -->|
 0             25                    60             85
```

- **P1**: meets deadline 50 at time 25, meets deadline 100 at time 85. ✓
- **P2**: meets deadline 80 at time 60. ✓

**All deadlines met** — EDF handled what RM couldn't by letting P2 keep the CPU when its deadline was closer.

### 5.10 Rate-Monotonic vs EDF

| Property | Rate-Monotonic | EDF |
|----------|---------------|-----|
| Priority | **Fixed** (period-based) | **Dynamic** (deadline-based) |
| CPU utilization bound | ~69.3% (as N → ∞) | **100%** (theoretical) |
| Implementation complexity | **Low** | High |
| Overhead | **Low** | High — recompute priority on each deadline event |
| Task types supported | Periodic tasks only | Both periodic and aperiodic |
| Optimality | Optimal among **fixed-priority** algorithms | **Theoretically optimal** overall |
| Practical use | Aviation, automotive — **predictability** valued | Less common — harder to analyze formally |

> **Why is RM preferred in safety-critical systems despite worse utilization?** Because RM is **predictable and easy to certify**: you can prove by hand that deadlines will be met if utilization is under the bound. EDF's dynamic priorities are harder to reason about formally, which is a liability when failure = people die.

### 5.11 Proportional Share Scheduling

Distribute a total of **T shares** among applications, and each gets CPU time in proportion to its shares.

**Example**: T = 100 shares.
- Process A: 50 shares → **50%** of CPU time.
- Process B: 15 shares → **15%** of CPU time.
- Process C: 20 shares → **20%** of CPU time.
- Remaining 15 shares → surplus.

**Admission Control**:
- A new process requests 30 shares.
- In use: 50 + 15 + 20 = 85; remaining: 15.
- 15 < 30 → **rejected**.

The scheduler only admits new work when **sufficient shares are available** → this provides a **guaranteed proportional** share of CPU time to all admitted processes. No process can starve another.

> **Modern cousins**: Linux cgroups' CPU quota / CFS weights, container CPU limits (Docker, Kubernetes), and hypervisor CPU shares (VMware) are all proportional-share schemes. They're how cloud providers guarantee "you get X% of a vCPU".

---

<br>

## 6. Operating System Examples

### 6.1 Linux CFS — Overview and History

**CFS (Completely Fair Scheduler)** — the default Linux scheduler since kernel **2.6.23** (2007).

**History:**
- Before Linux 2.5: traditional UNIX scheduler (simple, but poor SMP scaling).
- **Linux 2.5**: the **O(1) scheduler** — constant-time operations, good SMP support.
- **Linux 2.6.23+**: **CFS** — better interactive response, cleaner theory.

**Core goal**: divide CPU time "completely fairly" among runnable tasks, weighted by priority.

**Scheduling Classes** (Linux separates real-time and normal workloads):
- **Real-time class**: `SCHED_FIFO`, `SCHED_RR` — priorities 0–99 (higher priority than any normal task).
- **Normal class**: **CFS** — priorities 100–139, driven by the **nice** value (range −20 to +19).

### 6.2 Linux Priority Ranges

*Silberschatz, Figure 5.26 — Scheduling priorities on a Linux system*

- **Smaller number means higher priority** (follows textbook convention).
- Real-time tasks (0–99) **always preempt** normal tasks (100–139).
- Normal task priority = `100 + (nice + 20) = 120 + nice`.
  - `nice = −20` → priority 100 (highest normal).
  - `nice =   0` → priority 120 (default).
  - `nice = +19` → priority 139 (lowest).

> **Why "nice"?** Historically, a process being "nice" means it's *willing to be deprioritized* so others can run — hence "+nice = lower priority". The `nice` and `renice` commands let users adjust this from the shell.

### 6.3 CFS — vruntime Concept

CFS tracks **vruntime (virtual runtime)** — an abstract measure of how much CPU each task has "used up fairly".

- CFS always runs the **task with the smallest vruntime**.
- vruntime increases as a task runs, but **weighted by nice value**.

| nice value | vruntime increase rate |
|:----------:|:----------------------:|
| Low nice (−20, high priority) | Increases **slower** than real time |
| nice = 0 (default) | Increases at the **same** rate as real time |
| High nice (+19, low priority) | Increases **faster** than real time |

**Example**:
- A task with `nice = 0` runs for 200 ms real time → vruntime increases by ~200.
- A task with `nice = −5` runs for 200 ms real time → vruntime increases by less than 200.

Because CFS always picks the smallest vruntime, **high-priority tasks (slower-growing vruntime) get more real CPU time**.

### 6.4 CFS — vruntime Step-by-Step Example

Three tasks: A (nice = 0), B (nice = 0), C (nice = 5).
Initial vruntime: A = 0, B = 0, C = 0.

```text
 Step 1: A runs (10 ms)  -> A.vruntime = 10
         vruntimes: A=10, B=0, C=0  -> B has min (tied with C, pick B) -> B selected

 Step 2: B runs (10 ms)  -> B.vruntime = 10
         vruntimes: A=10, B=10, C=0  -> C has min -> C selected

 Step 3: C runs (10 ms)  -> C.vruntime = 15  (increases faster due to nice=5)
         vruntimes: A=10, B=10, C=15  -> A (tied with B, pick A) -> A selected

 Step 4: A runs (10 ms)  -> A.vruntime = 20
         vruntimes: A=20, B=10, C=15  -> B has min -> B selected
```

Observations:
- Higher-priority tasks accumulate vruntime slowly → they're picked more often.
- **I/O-bound tasks naturally win**: they run briefly, then block for I/O. While they're blocked, their vruntime **does not advance**, while other runnable tasks' vruntimes climb. When the I/O-bound task wakes up, its vruntime is below everyone else's → it runs immediately. No special "I/O boost" needed.

### 6.5 CFS — Target Latency and Time Slice

CFS **does not use a fixed time quantum**. Instead, it computes a time slice dynamically.

**Target Latency** — the time window in which every runnable task should get to run **once**.
- Example: target latency = 20 ms, with 4 tasks → each gets roughly 5 ms.

**Minimum Granularity** — a floor on how short a time slice can become.
- Prevents unreasonably short slices when many tasks are runnable.
- Example: minimum = 1 ms.

**Time slice calculation** — the target latency is **distributed in weighted proportions** according to nice values:
- High-priority (low nice) → **longer** time slice.
- Low-priority (high nice) → **shorter** time slice.

> **Why dynamic slices beat fixed quanta**: With a fixed quantum, adding 100 new tasks means 100× more context switches per unit time. With target latency, the slice shrinks until minimum granularity kicks in, capping the context-switch rate. This is why CFS scales smoothly from 2 tasks to 2000.

### 6.6 CFS — Red-Black Tree

CFS stores runnable tasks in a **Red-Black Tree** keyed by **vruntime**.

```text
                    T3 (vruntime=25)
                   /                \
           T1 (vruntime=15)    T5 (vruntime=35)
           /        \              /        \
    T0 (vr=10)  T2 (vr=20)  T4 (vr=30)  T6 (vr=40)
     ^
     |
   leftmost node
   = smallest vruntime
   = NEXT to run
```

- **Leftmost node** = smallest vruntime = next task to execute.
- The kernel caches this in a variable `rb_leftmost` → picking the next task is **O(1)**.
- Insertion/deletion: **O(log N)**.
- When a task **blocks** (e.g., waits for I/O), it's **removed** from the tree. When it becomes runnable again, it's **inserted** back.

> **Why a red-black tree and not a heap?** A heap gives O(1) min-find and O(log N) insert, same as the leftmost-cached RB-tree. But CFS also needs to **remove arbitrary elements** (when a task blocks) and **update priorities** — and RB-trees support these operations in O(log N) with cleaner invariants than heaps.

### 6.7 CFS — Load Balancing and NUMA

CFS load balancing uses a **scheduling domain hierarchy** to match the machine's memory topology.

*Silberschatz, Figure 5.27 — NUMA-aware load balancing with Linux CFS scheduler*

**Strategy — from low-cost to high-cost migrations:**
1. **Within a domain** (cores sharing L2 cache) — cheap, migrate freely.
2. **Across domains** (cores sharing L3 cache) — medium cost.
3. **Across NUMA nodes** (different memory banks) — expensive, **only under extreme imbalance**.

CFS defines **load = priority × CPU utilization ratio**, so a high-priority task counts more toward a CPU's "load" than a low-priority one.

> **Takeaway**: CFS isn't just "pick smallest vruntime" — on a real multi-socket server, it's a layered system where the single-CPU algorithm (vruntime + RB-tree) is wrapped by a multi-CPU balancer that respects cache and memory locality.

### 6.8 Windows Scheduling

**Priority-based preemptive scheduling**.

Priority structure: **32 levels**
- **0** — idle thread (memory-management placeholder).
- **1–15** — Variable class (normal user/system work).
- **16–31** — Real-time class.

**Dispatcher**: always runs the highest-priority ready thread.

Behavior details:
- **Same priority** → **Round-Robin** among them.
- **Variable class**: priority **decreases** when a thread's time quantum expires (but never below its base priority).
- **Priority boost** on events like I/O completion, keyboard input → interactive threads become responsive quickly.

### 6.9 Windows — Priority Classes

The Windows API groups priorities into **6 priority classes**, and within each class, **7 relative priority levels**.

| Priority Class | Base Priority |
|----------------|:-------------:|
| REALTIME_PRIORITY_CLASS | 24 |
| HIGH_PRIORITY_CLASS | 13 |
| ABOVE_NORMAL_PRIORITY_CLASS | 10 |
| NORMAL_PRIORITY_CLASS | 8 |
| BELOW_NORMAL_PRIORITY_CLASS | 6 |
| IDLE_PRIORITY_CLASS | 4 |

Within each class: IDLE, LOWEST, BELOW_NORMAL, **NORMAL**, ABOVE_NORMAL, HIGHEST, TIME_CRITICAL.

**Example**: `ABOVE_NORMAL_PRIORITY_CLASS` (base 10) + NORMAL relative = **priority 10**.

APIs:
- `SetPriorityClass()` — change a process's priority class.
- `SetThreadPriority()` — change the relative priority of a thread within its class.

### 6.10 Windows — Variable Priority Behavior

Windows adjusts thread priorities dynamically based on behavior:

```text
 Base Priority = 8 (NORMAL class, NORMAL relative)

 Time quantum exhausted
 --> priority decreases (e.g., 8 -> 7)
 --> never drops below base priority

 I/O completion (keyboard input)
 --> large priority boost (e.g., 8 -> 14)

 I/O completion (disk)
 --> medium priority boost (e.g., 8 -> 10)

 Foreground window
 --> time quantum is tripled
```

**Goal**: give **good response time to interactive threads** (they get big boosts when user events arrive) and **keep I/O devices busy** (I/O-completing threads are scheduled fast). Meanwhile, CPU-bound threads that exhaust their quantum drift toward their base priority — they still run, just not at the expense of interactivity.

### 6.11 Windows — Multiprocessor Support

**SMT Set** — a group of hardware threads that live on the **same core**.
- Example: 4 cores × 2 threads/core → SMT sets `{0,1}`, `{2,3}`, `{4,5}`, `{6,7}`.
- Keeping a thread inside its SMT set → **better cache efficiency** (shared L1/L2).

**Ideal Processor** — each thread is assigned a **preferred CPU number**:
- Starts from a per-process **seed value**, incremented as each new thread is created.
- Distributed across **different SMT sets**: 0, 2, 4, 6, 0, 2, ...
- Different processes use different seeds → threads are spread out evenly across cores system-wide.

> **Why not just "any idle CPU"?** Because on a multi-core system with SMT, *which* idle CPU matters: putting a new thread on a free SMT sibling shares L1 cache; putting it on a different socket incurs remote-memory costs. Ideal Processor codifies "prefer this CPU, then its SMT sibling, then nearby, then distant" as policy.

---

<br>

## 7. Algorithm Evaluation

### 7.1 Four Evaluation Methods — Overview

| Method | Description | Accuracy |
|--------|-------------|----------|
| **Deterministic Modeling** | Analytical calculation with a specific workload | Low (covers only specific cases) |
| **Queueing Models** | Mathematical models (Little's formula, etc.) | Medium (approximation) |
| **Simulation** | Computer simulation of the scheduler | High |
| **Implementation** | Implement in a real OS and measure | Highest (true environment) |

**Trade-off**: higher accuracy → higher cost and complexity.

### 7.2 Deterministic Modeling

Assume a **specific workload** and calculate exact performance for each algorithm.

| Process | Burst Time |
|---------|-----------|
| P1 | 10 |
| P2 | 29 |
| P3 | 3  |
| P4 | 7  |
| P5 | 12 |

All processes arrive at time 0.

| Algorithm | Average Waiting Time |
|-----------|---------------------|
| FCFS | (0 + 10 + 39 + 42 + 49) / 5 = **28 ms** |
| SJF | (10 + 32 + 0 + 3 + 20) / 5 = **13 ms** |
| RR (q = 10) | (0 + 32 + 20 + 23 + 40) / 5 = **23 ms** |

**Pros**: simple, fast, precise for the given workload.
**Cons**: conclusions apply **only to that specific workload** — real workloads vary.

### 7.3 Queueing Models — Little's Formula

Model the system as a **network of servers + queues**.

**Little's Formula:**

> **n = λ × W**

- **n** — average number of processes in the queue (queue length).
- **λ** — average arrival rate (processes per second).
- **W** — average waiting time (seconds per process).

**Example**: λ = 7 processes/sec, n = 14 processes.
- W = n / λ = 14 / 7 = **2 seconds**.

**Limitations**: Little's formula is a general identity, but computing `n` and `λ` for realistic scheduling policies requires distributions that are often **mathematically intractable**. Also, independence assumptions (arrivals / services uncorrelated) may not hold.

> **Why Little's formula is beautiful**: it requires no assumptions about the distribution of arrivals or service times — it's just conservation of flow. Whenever you know two of the three quantities, you can derive the third.

### 7.4 Simulation and Implementation

**Simulation**
- Build a software model of the computer system and run the scheduler against synthetic workloads.
- Workload sources:
  - **Random number generation** — convenient but may not match real usage.
  - **Trace files** — recorded sequences of events from real systems — reproduce real behavior exactly, much higher accuracy.
- **Cons**: development cost, long execution time, storage space for traces.

**Implementation**
- Actually implement the algorithm in a real OS and run it on real hardware with real users.
- The **only completely accurate** evaluation method.
- **Cons**:
  - High implementation and testing cost.
  - **User behavior changes with the environment** — once users notice the scheduler favors certain patterns, they may game it (e.g., splitting jobs to get the "short-job" discount).

> **A famous cautionary tale**: In an early multi-user OS, the scheduler boosted jobs that did lots of I/O. Users quickly discovered this and added useless I/O calls to their CPU-bound jobs to trick the scheduler. The evaluation that looked great in simulation collapsed in deployment. This is why real-world evaluation is irreplaceable.

---

<br>

## 8. Lab — MLFQ Simulator

### 8.1 Lab: Queue Configuration and Rules

Implement a **3-level Multilevel Feedback Queue** simulator.

**Queue configuration**:

| Queue | Algorithm | Time Quantum |
|:-----:|-----------|:------------:|
| Q0 | Round-Robin | 8 |
| Q1 | Round-Robin | 16 |
| Q2 | FCFS | None (run until completion) |

**Rules**:
- New processes enter **Q0**.
- If a process doesn't complete within Q0's quantum → demoted to **Q1**.
- If it doesn't complete within Q1's quantum → demoted to **Q2**.
- **Aging**: if a process waits more than a threshold (e.g., 30 time units) in Q2, promote it back to **Q0**.

### 8.2 Lab: Data Structure and Skeleton

```python
from collections import deque

class Process:
    def __init__(self, pid, arrival, burst):
        self.pid = pid
        self.arrival = arrival
        self.burst = burst
        self.remaining = burst
        self.queue_level = 0
        self.wait_time = 0
        self.start_time = -1
        self.finish_time = -1

def mlfq_scheduler(processes, quanta=[8, 16]):
    queues = [deque(), deque(), deque()]
    time = 0
    completed = []
    gantt = []

    while not all_done(processes, completed):
        # 1. Insert arriving processes into Q0
        # 2. Check aging: promote processes that have waited too long in Q2 back to Q0
        # 3. Select a runnable process in order Q0 -> Q1 -> Q2
        # 4. Execute according to the selected queue's algorithm
        # 5. On quantum exhaustion demote; on completion record finish time
        # 6. Record Gantt chart and statistics
        pass

    return gantt, compute_statistics(completed)
```

### 8.3 Lab: Core Execution Logic

```python
def run_process(proc, queue_level, quanta, time):
    """Run the selected process for the queue's quantum."""
    if queue_level < 2:            # Q0 or Q1 -> RR
        quantum = quanta[queue_level]
        run_time = min(proc.remaining, quantum)
    else:                          # Q2 -> FCFS, no quantum
        run_time = proc.remaining

    proc.remaining -= run_time
    new_time = time + run_time

    if proc.remaining == 0:
        # Process completed
        proc.finish_time = new_time
        return new_time, 'completed'
    else:
        # Quantum exhausted -> demote
        proc.queue_level = min(queue_level + 1, 2)
        return new_time, 'demoted'
```

Test data: `P1(arrival=0, burst=30), P2(2, 12), P3(4, 6), P4(6, 20), P5(8, 4)`.

### 8.4 Lab: Sample Output

Gantt chart:

```text
| P1(Q0) | P2(Q0) | P3(Q0) | P4(Q0) | P5(Q0) | P2(Q1) | P4(Q1) | P1(Q1) | P1(Q2) |
0        8       16       22       30       34       38       50       66       72
```

Statistics:

| Process | Arrival | Burst | Finish | Turnaround | Waiting |
|---------|:-------:|:-----:|:------:|:----------:|:-------:|
| P1 | 0 | 30 | 72 | 72 | 42 |
| P2 | 2 | 12 | 38 | 36 | 24 |
| P3 | 4 | 6  | 22 | 18 | 12 |
| P4 | 6 | 20 | 50 | 44 | 24 |
| P5 | 8 | 4  | 34 | 26 | 22 |

- **Average Turnaround** = (72 + 36 + 18 + 44 + 26) / 5 = **39.2 ms**.
- **Average Waiting** = (42 + 24 + 12 + 24 + 22) / 5 = **24.8 ms**.

Notice how short jobs (P3, P5) finish entirely in Q0 (top priority, fast response), while the longest job (P1) sinks to Q2 and finishes last — this is MLFQ favoring short processes as designed.

---

<br>

## Summary

| Concept | Key Summary |
|:--------|:-----------|
| Priority Scheduling | Priority-based (smaller = higher); starvation solved with **Aging** |
| Multilevel Queue | Split into multiple queues with different algorithms; **fixed assignment** |
| MLFQ | Movement **between queues** via demotion (quantum exhaustion) and promotion (aging); auto-classifies I/O- vs CPU-bound |
| Thread Scheduling | **PCS** (intra-process, library) vs **SCS** (system-wide, kernel); modern OSes use SCS only |
| SMP | Per-processor queues + load balancing; asymmetric master is bottleneck |
| Processor Affinity | **Soft** (OS tries) vs **Hard** (explicit pin); conflicts with load balancing |
| NUMA | Local vs remote memory; place process near its memory; CFS scheduling domain hierarchy |
| Multicore / CMT | Memory stall → SMT/Hyper-Threading keeps cores busy; two-level scheduling (OS + core hardware) |
| HMP (big.LITTLE) | Same ISA, different power/perf; big = interactive, LITTLE = background |
| Real-Time | Soft (best-effort) vs Hard (must meet deadline); needs preemptive kernel + bounded latency |
| Rate-Monotonic | **Fixed priority** = 1/period; bound ≈ 69.3% (N→∞); optimal among fixed-priority |
| EDF | **Dynamic priority** = closest deadline; theoretically schedulable up to 100%; handles aperiodic tasks |
| Proportional Share | Distribute **T shares**; admission control rejects if insufficient shares |
| Linux CFS | **vruntime** + **Red-Black Tree**; target latency instead of fixed quantum; NUMA-aware |
| Windows | 32 priorities; Variable (1–15) + Real-time (16–31); priority boost on I/O; Ideal Processor + SMT sets |
| Algorithm Evaluation | Deterministic → Queueing (Little's n = λW) → Simulation → Implementation |
| Textbook Scope | Silberschatz Ch 5, Sections 5.3.4 – 5.8 |

---

<br>

## Self-Check Questions

1. What is priority scheduling, and how does it relate to SJF? Which convention does Silberschatz use — smaller or larger number for higher priority?
2. What is starvation in priority scheduling? Describe aging and explain precisely how it resolves starvation. Given priorities 0 (high) to 127 (low) and a 1-unit-per-second increase, how long does a priority-127 process take to reach the highest priority?
3. Compare Multilevel Queue and Multilevel Feedback Queue (MLFQ). What exactly is the key difference, and why does it matter? List the five MLFQ design parameters.
4. In the MLFQ example from the lecture (Q0: RR q=8, Q1: RR q=16, Q2: FCFS), classify a process with burst = 12 ms and burst = 30 ms — which queues will each visit, and when does each complete (ignore contention)?
5. Distinguish PCS and SCS in thread scheduling. Why do Linux and macOS only support PTHREAD_SCOPE_SYSTEM?
6. Compare the common-ready-queue and per-processor-queue designs in SMP. Which do modern systems use, and how do they address the downsides of that choice?
7. What is processor affinity, and why does it exist? Distinguish soft affinity and hard affinity. How does affinity conflict with load balancing, and how is the conflict resolved?
8. Explain NUMA and why NUMA-aware scheduling matters for performance. How does Linux CFS handle NUMA?
9. State the periodic task model (t, d, p). For P1(t=20, p=50) and P2(t=35, p=100), compute total CPU utilization and decide whether RM will succeed (show the utilization bound).
10. Given P1(t=25, p=50) and P2(t=35, p=80), show that RM fails but EDF succeeds. Walk through the time-step reasoning for both algorithms.
11. Compare RM and EDF on: priority type, utilization bound, overhead, optimality. Why is RM still preferred in safety-critical systems despite lower utilization?
12. Explain the vruntime mechanism in CFS. Why does an I/O-bound task naturally get good response time without any explicit I/O boost?
13. Why does CFS use a red-black tree keyed by vruntime? What operations does it need to support, and what are their complexities?
14. Describe Windows priority classes and the priority boost mechanism. Why does Windows boost the priority of a thread that just completed I/O?
15. Name the four scheduling-algorithm evaluation methods in order of increasing cost and accuracy. State Little's formula and explain what each symbol means.

---
