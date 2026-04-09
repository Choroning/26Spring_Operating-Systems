# Week 6 Concepts Lecture — CPU Scheduling (1)

> **Last Updated:** 2026-04-09
>
> Silberschatz, Operating System Concepts Ch 5 (Sections 5.1 – 5.3)

> **Prerequisites**: Week 5 Thread and Concurrency concepts. Understanding of process states (ready, running, waiting) and context switching.
>
> **Learning Objectives**:
> 1. Understand the CPU-I/O Burst Cycle and explain the need for scheduling
> 2. Distinguish between Preemptive vs Nonpreemptive scheduling
> 3. Explain the role of the Dispatcher and Dispatch Latency
> 4. List and explain the 5 Scheduling Criteria
> 5. Understand FCFS, SJF, SRTF, RR algorithms and draw Gantt Charts
> 6. Perform CPU burst prediction using Exponential Averaging

---

## Table of Contents

- [1. Basic Concepts](#1-basic-concepts)
  - [1.1 Multiprogramming and CPU Scheduling](#11-multiprogramming-and-cpu-scheduling)
  - [1.2 CPU-I/O Burst Cycle](#12-cpu-io-burst-cycle)
  - [1.3 CPU Burst Distribution](#13-cpu-burst-distribution)
  - [1.4 I/O-bound vs CPU-bound Process](#14-io-bound-vs-cpu-bound-process)
  - [1.5 CPU Scheduler (Short-term Scheduler)](#15-cpu-scheduler-short-term-scheduler)
  - [1.6 When Scheduling Occurs — 4 Circumstances](#16-when-scheduling-occurs--4-circumstances)
  - [1.7 Nonpreemptive vs Preemptive Scheduling](#17-nonpreemptive-vs-preemptive-scheduling)
  - [1.8 Issues with Preemptive Scheduling](#18-issues-with-preemptive-scheduling)
  - [1.9 Dispatcher](#19-dispatcher)
  - [1.10 Dispatch Latency](#110-dispatch-latency)
- [2. Scheduling Criteria](#2-scheduling-criteria)
  - [2.1 Scheduling Criteria — Overview](#21-scheduling-criteria--overview)
  - [2.2 CPU Utilization and Throughput](#22-cpu-utilization-and-throughput)
  - [2.3 Turnaround Time and Waiting Time](#23-turnaround-time-and-waiting-time)
  - [2.4 Response Time](#24-response-time)
  - [2.5 Scheduling Criteria — Summary](#25-scheduling-criteria--summary)
- [3. FCFS Scheduling](#3-fcfs-scheduling)
  - [3.1 FCFS (First-Come, First-Served)](#31-fcfs-first-come-first-served)
  - [3.2 FCFS — Example 1 (Arrival Order: P1, P2, P3)](#32-fcfs--example-1-arrival-order-p1-p2-p3)
  - [3.3 FCFS — Example 2 (Arrival Order: P2, P3, P1)](#33-fcfs--example-2-arrival-order-p2-p3-p1)
  - [3.4 FCFS — Convoy Effect](#34-fcfs--convoy-effect)
  - [3.5 FCFS — Characteristics Summary](#35-fcfs--characteristics-summary)
- [4. SJF Scheduling](#4-sjf-scheduling)
  - [4.1 SJF (Shortest-Job-First)](#41-sjf-shortest-job-first)
  - [4.2 SJF — Proof of Optimality](#42-sjf--proof-of-optimality)
  - [4.3 SJF — Example (Nonpreemptive)](#43-sjf--example-nonpreemptive)
  - [4.4 Predicting the Next Burst Length](#44-predicting-the-next-burst-length)
  - [4.5 Exponential Averaging — Formula](#45-exponential-averaging--formula)
  - [4.6 Exponential Averaging — Expansion](#46-exponential-averaging--expansion)
  - [4.7 Exponential Averaging — Calculation Example](#47-exponential-averaging--calculation-example)
  - [4.8 Exponential Averaging — Comparing Alpha Values](#48-exponential-averaging--comparing-alpha-values)
- [5. SRTF — Preemptive SJF](#5-srtf--preemptive-sjf)
  - [5.1 SRTF (Shortest-Remaining-Time-First)](#51-srtf-shortest-remaining-time-first)
  - [5.2 SRTF — Step-by-Step Example](#52-srtf--step-by-step-example)
  - [5.3 SRTF — Gantt Chart and Calculation](#53-srtf--gantt-chart-and-calculation)
  - [5.4 SJF vs SRTF — Same Data Comparison](#54-sjf-vs-srtf--same-data-comparison)
- [6. Round-Robin Scheduling](#6-round-robin-scheduling)
  - [6.1 Round-Robin (RR)](#61-round-robin-rr)
  - [6.2 RR — Fairness Guarantee](#62-rr--fairness-guarantee)
  - [6.3 RR — Example (q = 4ms)](#63-rr--example-q--4ms)
  - [6.4 RR — Time Quantum Too Large or Too Small](#64-rr--time-quantum-too-large-or-too-small)
  - [6.5 RR — Time Quantum and Turnaround Time](#65-rr--time-quantum-and-turnaround-time)
  - [6.6 RR — Detailed Example (4 Processes)](#66-rr--detailed-example-4-processes)
- [7. Algorithm Comparison](#7-algorithm-comparison)
  - [7.1 Four Algorithms on the Same Data](#71-four-algorithms-on-the-same-data)
  - [7.2 Comprehensive Characteristics Comparison](#72-comprehensive-characteristics-comparison)
- [8. Lab — Scheduling Algorithm Simulator](#8-lab--scheduling-algorithm-simulator)
  - [8.1 Lab: Data Structure](#81-lab-data-structure)
  - [8.2 Lab: FCFS Implementation](#82-lab-fcfs-implementation)
  - [8.3 Lab: SJF Implementation (Nonpreemptive)](#83-lab-sjf-implementation-nonpreemptive)
  - [8.4 Lab: RR Implementation](#84-lab-rr-implementation)
  - [8.5 Lab: Testing and Comparing Results](#85-lab-testing-and-comparing-results)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. Basic Concepts

### 1.1 Multiprogramming and CPU Scheduling

**Key Idea**: Keep the CPU busy at all times to **maximize utilization**

- In a single CPU core system, only one process can run at a time
- While a process waits for I/O, the CPU is **idle** — wasted
- **Multiprogramming**: Keep multiple processes in memory; when one waits, allocate the CPU to another process
- CPU scheduling is one of the **most fundamental functions** of an operating system

> Think of CPU scheduling like a restaurant kitchen with one stove (CPU). If a chef puts a dish in the oven (I/O burst) and just stands there waiting, that's wasted time. A good kitchen manager (scheduler) assigns the chef to start preparing another dish (CPU burst for another process) while the first one bakes. Multiprogramming ensures the stove is always in use.

### 1.2 CPU-I/O Burst Cycle

Process execution consists of a repeating cycle of **CPU bursts** and **I/O bursts**:

- **CPU burst**: The interval during which the CPU executes instructions
- **I/O burst**: The interval during which the process waits for I/O completion
- In the final CPU burst, the process requests **termination via a system call**

```c
// Process execution pattern (pseudocode)
while (process_not_terminated) {
    // === CPU Burst ===
    execute_instructions();   // load, store, add, ...

    // === I/O Burst ===
    request_io();             // read from file, write to file, ...
    wait_for_io_completion(); // blocking
}
terminate();                  // Terminates after the last CPU burst
```

- All processes follow this pattern, but the **burst length and frequency** differ
- This difference is the key basis for choosing scheduling algorithms

> The CPU-I/O burst cycle is the fundamental rhythm of process execution. Every process alternates between "thinking" (CPU burst) and "waiting" (I/O burst). The scheduling algorithm's job is to efficiently fill the gaps — when one process enters an I/O burst, another process should be ready for its CPU burst.

### 1.3 CPU Burst Distribution

The distribution of CPU burst lengths follows an **exponential or hyperexponential** (hyperexponential — a distribution with a heavier tail than a plain exponential, meaning bursts cluster even more strongly at the short end) decay pattern:

- **Short CPU bursts** are very frequent — most bursts are short
- **Long CPU bursts** occur rarely

*Silberschatz, Figure 5.2 — Histogram of CPU-burst durations*

> This distribution is crucial because it validates the design of algorithms like SJF: since most bursts are short, prioritizing short bursts means most processes get served quickly, and only the rare long-burst processes wait longer.

### 1.4 I/O-bound vs CPU-bound Process

Process classification based on CPU burst distribution:

| Category | I/O-bound Process | CPU-bound Process |
|----------|-------------------|-------------------|
| CPU burst | Many **short** bursts | Few **long** bursts |
| I/O burst | Frequent I/O requests | Rare I/O requests |
| Examples | Web server, database | Scientific computation, video encoding |
| Scheduling needs | Fast response time | Long CPU allocation time |

- **Scheduling algorithms** must efficiently handle both types
- This distribution characteristic forms the theoretical basis for algorithms like SJF and RR

> **Exam Tip:** Being able to classify processes as I/O-bound or CPU-bound is essential for understanding why different scheduling algorithms favor different workloads. FCFS favors CPU-bound processes (they get long uninterrupted runs), while RR favors I/O-bound processes (they get quick, frequent access to the CPU).

### 1.5 CPU Scheduler (Short-term Scheduler)

When the CPU becomes idle, the **ready queue** is used to select the next process to run:

```text
                    +-------------+
  new process --->  | Ready Queue | ---> CPU Scheduler ---> CPU
                    +-------------+   selection algorithm
                          ^
                          |
                  I/O completion, etc.
```

- **CPU Scheduler** = Short-term Scheduler
- The ready queue does not have to be FIFO
  - Can be implemented as a FIFO queue, **priority queue**, tree, linked list, etc.
- Records in the queue are typically **PCBs (Process Control Blocks)**
  - Recall: A PCB (Process Control Block) stores all state information for a process — registers, program counter, priority, etc. — so that it can be resumed exactly where it left off after a context switch.
- The internal order of the queue is determined by the scheduling algorithm

> **Key Point:** The "ready queue" is not necessarily a queue in the data structure sense. It's a collection of processes that are ready to run. The scheduling algorithm determines how that collection is ordered — FCFS uses a FIFO queue, SJF uses a priority queue sorted by burst length, etc. The name "ready queue" is a historical convention.

### 1.6 When Scheduling Occurs — 4 Circumstances

**4 circumstances** where CPU scheduling decisions are needed:

| No. | State Transition | Example | Choice |
|-----|------------------|---------|--------|
| **1** | Running -> Waiting | I/O request, wait() call | No choice |
| **2** | Running -> Ready | Interrupt occurs (timer, etc.) | **Choice exists** |
| **3** | Waiting -> Ready | I/O completion | **Choice exists** |
| **4** | Terminates | Process terminates | No choice |

- Circumstances **1, 4**: A new process must be selected (no choice)
- Circumstances **2, 3**: Depends on the scheduling scheme (choice exists)

> Understanding these four circumstances is fundamental to understanding the difference between preemptive and nonpreemptive scheduling. Circumstances 1 and 4 are mandatory scheduling points — the running process can no longer use the CPU. Circumstances 2 and 3 are optional — they only trigger scheduling decisions in preemptive systems.

### 1.7 Nonpreemptive vs Preemptive Scheduling

| Category | Nonpreemptive | Preemptive |
|----------|---------------|------------|
| Scheduling points | Only at **1, 4** | At **1, 2, 3, 4** (all) |
| CPU release | Process **voluntarily** releases | OS can **forcibly** take away |
| Also known as | Cooperative scheduling | - |
| Usage | Early OS | **Most modern OS** |

- **Nonpreemptive**: Process voluntarily releases the CPU (termination or I/O wait)
- **Preemptive**: OS can forcibly reclaim the CPU via timer interrupt, etc.
- Windows, macOS, Linux, UNIX — all use **preemptive** scheduling

> **Exam Tip:** "Nonpreemptive = circumstances 1 and 4 only" and "Preemptive = all four circumstances" is a frequently tested distinction. Remember that nonpreemptive is also called "cooperative" because the process cooperates by voluntarily giving up the CPU.

### 1.8 Issues with Preemptive Scheduling

Preemptive scheduling is powerful but has **caveats**:

**1. Race Condition**
- Preemption while modifying shared data -> another process reads inconsistent data
- -> **Synchronization mechanisms** (mutex, semaphore, etc.) are needed (covered in detail in Chapter 6)

A **race condition** occurs when two processes read and write shared data at the same time, potentially leaving it in an invalid state. A **mutex** (mutual exclusion lock) prevents this by allowing only one process at a time into a **critical section** — the portion of code that accesses shared data.

**2. Preemption in Kernel Mode**
- Preemption during kernel data structure modification within a system call -> kernel inconsistency
- **Nonpreemptive kernel**: Defer context switch until system call completes (simple but unsuitable for real-time)
- **Preemptive kernel**: Protect kernel data with mutex locks, etc. (most modern OS)

**3. Interrupt Handling**
- Interrupts can occur at any time -> disable interrupts in critical sections

> These issues foreshadow the synchronization topics in Week 9. The key insight is that preemption introduces concurrency problems: if the OS can stop a process at any point, then shared data might be in an inconsistent state when another process reads it. This is the fundamental reason why synchronization mechanisms exist.

### 1.9 Dispatcher

**Dispatcher**: The module that actually hands over CPU control to the process selected by the Scheduler

Three roles of the Dispatcher:
1. Perform **context switch** (save current process state -> restore new process state)
2. Switch to **user mode**
3. Jump to the appropriate location (PC — Program Counter, the register holding the address of the next instruction) of the new process

*Silberschatz, Figure 5.3 — The role of the dispatcher*

> **Scheduler vs Dispatcher**: The scheduler **decides** which process runs next; the dispatcher **executes** that decision. Think of the scheduler as a manager who picks which employee works next, and the dispatcher as the HR person who handles the actual paperwork and office reassignment. The distinction matters because they are separate components with different responsibilities.

### 1.10 Dispatch Latency

**Dispatch Latency** = The time it takes for the Dispatcher to stop one process and start running another

- Occurs on every context switch, so it should be **as short as possible**

Checking context switch counts on Linux:

```bash
# System-wide context switch count (1-second interval, 3 times)
vmstat 1 3

# Context switch count for a specific process
cat /proc/<PID>/status | grep ctxt
# voluntary_ctxt_switches:    150   (voluntary: I/O wait, etc.)
# nonvoluntary_ctxt_switches: 8     (involuntary: time slice expired, etc.)
```

- **Voluntary context switch**: Process voluntarily releases the CPU due to lack of resources
- **Nonvoluntary context switch**: OS forces a switch due to time slice expiration, etc.

> **[Systems Programming]** On modern systems, a context switch typically takes around 1-10 microseconds. While this seems tiny, it adds up: a busy server performing thousands of context switches per second can lose significant CPU time to dispatch latency. This is why the time quantum in Round-Robin scheduling must be chosen carefully — too small a quantum means too many context switches.

---

<br>

## 2. Scheduling Criteria

### 2.1 Scheduling Criteria — Overview

Five criteria for **comparing** and **evaluating** scheduling algorithms:

| Criterion | Description | Optimization Direction |
|-----------|-------------|----------------------|
| **CPU Utilization** | CPU usage rate (%) | **Maximize** |
| **Throughput** | Number of processes completed per unit time | **Maximize** |
| **Turnaround Time** | Total time from submission to completion | **Minimize** |
| **Waiting Time** | Total time spent waiting in the ready queue | **Minimize** |
| **Response Time** | Time from submission to first response | **Minimize** |

### 2.2 CPU Utilization and Throughput

**CPU Utilization**
- The proportion of time the CPU is performing useful work
- Theoretical range: 0% to 100%
- Real systems: around **40%** under light load, **90%** under heavy load
- Can be checked with the `top` command on Linux

**Throughput**
- Number of processes completed per unit time
- Long processes: one every few seconds
- Short transactions: dozens per second

- The goal is to **maximize** both criteria
- They can be in a trade-off relationship (context switch overhead)

### 2.3 Turnaround Time and Waiting Time

**Turnaround Time**
- Total time from process **submission** to **completion**
- = Sum of ready queue wait + CPU execution + I/O wait time

```text
Turnaround Time = Completion Time - Arrival Time
```

**Waiting Time**
- Total time spent **waiting** in the ready queue
- The only time that the scheduling algorithm **directly affects**
- CPU execution time and I/O time are independent of the algorithm

```text
Waiting Time = Turnaround Time - Burst Time
             (for a single burst, with no I/O)
```

> **Exam Tip:** Waiting time is the most commonly used metric for comparing scheduling algorithms in textbook problems because it isolates the effect of the scheduling algorithm. Turnaround time includes CPU execution and I/O time, which are the same regardless of which algorithm is used. Waiting time captures *only* the delay introduced by the scheduling decision.

### 2.4 Response Time

**Response Time**
- Time from process **submission** to the moment the **first response is produced**
- The most important criterion in interactive systems

```text
Response Time = First Run Time - Arrival Time
```

**Turnaround Time vs Response Time**:

| Comparison | Turnaround Time | Response Time |
|------------|-----------------|---------------|
| Measured interval | Submission to **completion** | Submission to **first response** |
| Important for | Batch system | **Interactive** system |
| Example | 30 seconds to complete the job | 0.5 seconds to first output |

- In interactive systems, **minimizing the variance** of response time is important
- **Predictable** response time is more favorable for user satisfaction than average

### 2.5 Scheduling Criteria — Summary

```text
+------------------------------------------------+
|                    Maximize                    |
|  +-------------------+  +-------------------+  |
|  | CPU Utilization   |  |    Throughput     |  |
|  +-------------------+  +-------------------+  |
+------------------------------------------------+
|                    Minimize                    |
|  +-------------------+  +-------------------+  |
|  |  Turnaround Time  |  |   Waiting Time    |  |
|  +-------------------+  +-------------------+  |
|  +-------------------+                         |
|  |   Response Time   |                         |
|  +-------------------+                         |
+------------------------------------------------+
```

- In most cases, the **average** is optimized
- In some cases, the **minimum** or **maximum** is optimized (e.g., minimizing the maximum response time)

---

<br>

## 3. FCFS Scheduling

### 3.1 FCFS (First-Come, First-Served)

The **simplest** CPU scheduling algorithm: execute in **arrival order**

- Implemented with a **FIFO queue**
- When a process arrives at the ready queue, it is added to the **tail**
- When the CPU is free, it is allocated to the process at the **head**
- **Nonpreemptive** scheduling

```text
Ready Queue (FIFO):

  head ---> [ P1 ] ---> [ P2 ] ---> [ P3 ] <--- tail
              |
              v
            CPU allocation
```

### 3.2 FCFS — Example 1 (Arrival Order: P1, P2, P3)

Process arrival order: P1 -> P2 -> P3 (all arrive at time 0)

| Process | Burst Time |
|---------|-----------|
| P1 | 24 ms |
| P2 | 3 ms |
| P3 | 3 ms |

A **Gantt chart** in scheduling is a horizontal bar diagram where each bar shows which process is using the CPU during a given time interval; the numbers along the bottom are timestamps.

```text
Gantt Chart:
+-------------------------+----+----+
|            P1           | P2 | P3 |
+-------------------------+----+----+
0                         24   27   30
```

| Process | Waiting Time | Turnaround Time |
|---------|-------------|----------------|
| P1 | 0 | 24 |
| P2 | 24 | 27 |
| P3 | 27 | 30 |
| **Average** | **(0+24+27)/3 = 17** | **(24+27+30)/3 = 27** |

### 3.3 FCFS — Example 2 (Arrival Order: P2, P3, P1)

Same processes but different arrival order: P2 -> P3 -> P1

```text
Gantt Chart:
+----+----+-------------------------+
| P2 | P3 |            P1           |
+----+----+-------------------------+
0    3    6                         30
```

| Process | Waiting Time | Turnaround Time |
|---------|-------------|----------------|
| P1 | 6 | 30 |
| P2 | 0 | 3 |
| P3 | 3 | 6 |
| **Average** | **(6+0+3)/3 = 3** | **(30+3+6)/3 = 13** |

- Average Waiting Time: **17 -> 3** (dramatically reduced just by changing the order!)
- FCFS performance **varies significantly** depending on arrival order

### 3.4 FCFS — Convoy Effect

**Convoy Effect**: A phenomenon where short processes queue up behind a single process with a long CPU burst

```text
Scenario: 1 CPU-bound process + multiple I/O-bound processes

Time ->  ==========================================================>

CPU:  [ CPU-bound (long burst) ..........][ I/O-1 ][ I/O-2 ][ I/O-3 ]
I/O:  [  idle (I/O device sitting idle)  ][ busy  ][ busy  ][ busy  ]
                                         ^
                                         |
                                I/O-bound processes
                              waiting behind CPU-bound
```

- The CPU-bound process monopolizes the CPU -> I/O-bound processes wait in the ready queue
- Meanwhile, the **I/O device is idle** -> both CPU and I/O **utilization drops**
- **Unsuitable for interactive systems**: a single process can monopolize the CPU for a long time

> The convoy effect is named by analogy with a slow truck on a single-lane road: all faster cars (short processes) must wait behind it. In scheduling terms, this means both the CPU and I/O devices are underutilized — the I/O device sits idle while I/O-bound processes wait for the CPU, and the CPU sits idle later when all the I/O-bound processes do their I/O simultaneously.

### 3.5 FCFS — Characteristics Summary

| Characteristic | Description |
|----------------|-------------|
| Type | **Nonpreemptive** |
| Implementation | FIFO queue — very simple |
| Average Waiting Time | Generally **long** (not optimal) |
| Convoy Effect | **Occurs** (CPU-bound blocks I/O-bound) |
| Interactive suitability | **Unsuitable** (cannot distribute CPU at regular intervals) |
| Advantage | Simple to implement, easy to understand |
| Disadvantage | Performance heavily depends on arrival order |

---

<br>

## 4. SJF Scheduling

### 4.1 SJF (Shortest-Job-First)

Select the process with the **shortest next CPU burst** first:

- More accurate name: **Shortest-Next-CPU-Burst** Algorithm
- If burst lengths are equal -> tie-breaking with **FCFS**
- Both Nonpreemptive and Preemptive versions exist

**Key property**: Guarantees **minimum average waiting time** for a given set of processes — **Optimal**

**Fundamental problem**: The next CPU burst length **cannot be known exactly**

### 4.2 SJF — Proof of Optimality

**Why is SJF optimal?**

```text
Case 1: Long first (burst: 10, 3)
+-----------+----+
|   P1(10)  | P2 |   P1 wait=0, P2 wait=10  -> average = 5
+-----------+----+
0           10   13

Case 2: Short first (burst: 3, 10)  <- SJF
+----+-----------+
| P2 |   P1(10)  |   P2 wait=0, P1 wait=3   -> average = 1.5
+----+-----------+
0    3           13
```

- Moving the **shorter process** forward: its waiting time **decreases significantly**
- The **longer process's** waiting time **increases slightly**
- Result: The overall average waiting time **decreases**
- Generalizing this shows that SJF is **provably optimal**

> **[Algorithms]** The proof works by showing that in any non-SJF schedule, you can always swap two adjacent jobs to reduce total waiting time — repeating such swaps eventually yields the SJF order (this technique is called the exchange argument). In any schedule that is not SJF-ordered, you can find an adjacent pair where a longer job precedes a shorter one. Swapping them reduces total waiting time without affecting other jobs — a sorting argument that's equivalent to proving that sorting by burst time minimizes total weighted completion time.

### 4.3 SJF — Example (Nonpreemptive)

All processes arrive at time 0 (nonpreemptive SJF):

| Process | Burst Time |
|---------|-----------|
| P1 | 6 ms |
| P2 | 8 ms |
| P3 | 7 ms |
| P4 | 3 ms |

```text
Gantt Chart (SJF order: P4 -> P1 -> P3 -> P2):
+----+--------+----------+------------+
| P4 |   P1   |    P3    |     P2     |
+----+--------+----------+------------+
0    3        9         16            24
```

| Process | Waiting Time |
|---------|-------------|
| P1 | 3 |
| P2 | 16 |
| P3 | 9 |
| P4 | 0 |
| **Average** | **(3+16+9+0)/4 = 7** |

If FCFS were used? (P1->P2->P3->P4 order): average = (0+6+14+21)/4 = **10.25**

### 4.4 Predicting the Next Burst Length

SJF is **optimal** but impossible to implement in practice — because the **next CPU burst length cannot be known**

Solution: **Prediction** based on past burst history

- Past CPU bursts tend to repeat with similar lengths
- Use **Exponential Averaging** for prediction

### 4.5 Exponential Averaging — Formula

The idea is to form a **weighted average** between the most recent observed burst and the previous prediction, so that neither history nor the latest measurement is ignored entirely.

Formula for predicting the next CPU burst:

```text
tau(n+1) = alpha * t(n) + (1 - alpha) * tau(n)
```

| Symbol | Meaning |
|--------|---------|
| `t(n)` | The n-th **actual** CPU burst length (most recent observed value) |
| `tau(n)` | The n-th **predicted value** (previous prediction) |
| `tau(n+1)` | The next CPU burst **predicted value** |
| `alpha` | Weight (0 <= alpha <= 1) |

**Characteristics depending on alpha:**

| alpha value | Meaning |
|-------------|---------|
| alpha = 0 | `tau(n+1) = tau(n)` — Ignores recent observation, uses only past prediction |
| alpha = 1 | `tau(n+1) = t(n)` — Uses only the last observation, ignores past prediction |
| alpha = 1/2 | **Balances** recent and past values (common choice) |

### 4.6 Exponential Averaging — Expansion

Expanding the formula by repeated substitution:

```text
tau(n+1) = alpha * t(n)
         + (1-alpha) * alpha * t(n-1)
         + (1-alpha)^2 * alpha * t(n-2)
         + ...
         + (1-alpha)^j * alpha * t(n-j)
         + ...
         + (1-alpha)^(n+1) * tau(0)
```

- If alpha < 1, then **(1 - alpha)** is also < 1 -> weights **decrease exponentially** going further into the past
- The most recent observed value has the **greatest influence** on the prediction
- This is the origin of the name **"exponential averaging"**: the weights decay exponentially

> **[Probability/Statistics]** Exponential averaging is a special case of the Exponentially Weighted Moving Average (EWMA) used extensively in signal processing and time series analysis. The parameter alpha controls the "memory" of the estimator: higher alpha means shorter memory (responds quickly to changes), lower alpha means longer memory (more stable predictions). In networking, TCP uses a similar approach to estimate round-trip time.

### 4.7 Exponential Averaging — Calculation Example

Starting with alpha = 0.5, tau(0) = 10:

| n | t(n) | tau(n) | Calculation |
|---|------|--------|-------------|
| 0 | 6 | **10** | Initial value |
| 1 | 4 | **8** | 0.5*6 + 0.5*10 |
| 2 | 6 | **6** | 0.5*4 + 0.5*8 |
| 3 | 4 | **6** | 0.5*6 + 0.5*6 |
| 4 | 13 | **5** | 0.5*4 + 0.5*6 |
| 5 | 13 | **9** | 0.5*13 + 0.5*5 |
| 6 | 13 | **11** | 0.5*13 + 0.5*9 |
| 7 | - | **12** | 0.5*13 + 0.5*11 |

*Silberschatz, Figure 5.4 — Prediction of the length of the next CPU burst*

- Even when the actual burst changes suddenly, the predicted value gradually follows
- When the same value repeats consecutively, the prediction converges

### 4.8 Exponential Averaging — Comparing Alpha Values

Prediction differences for varying alpha on the same burst sequence (tau_0 = 10, bursts = [6, 4, 6]):

```text
               alpha = 0.2    alpha = 0.5    alpha = 0.8
tau(0) = 10        10             10             10
t(0) = 6
tau(1)             9.2            8.0            6.8
t(1) = 4
tau(2)             8.16           6.0            4.56
t(2) = 6
tau(3)             7.73           6.0            5.71
```

- **Smaller alpha**: Predicted value changes **slowly** (more dependent on history)
- **Larger alpha**: Predicted value **reacts quickly** to recent values
- In practice, values around **alpha = 0.5** are commonly used

---

<br>

## 5. SRTF — Preemptive SJF

### 5.1 SRTF (Shortest-Remaining-Time-First)

**Preemptive version of SJF** = SRTF

How it works:
- **Comparison is performed each time** a new process arrives
- If the new process's burst < the current process's **remaining burst** -> **preempt (switch)**
- Otherwise, the current process continues execution

| Comparison | SJF (Nonpreemptive) | SRTF (Preemptive) |
|------------|--------------------|--------------------|
| Comparison point | Only when CPU is free | **Each time** a new process arrives |
| Comparison target | Total burst length | **Remaining** burst length |
| Preemption | None | Yes |

### 5.2 SRTF — Step-by-Step Example

| Process | Arrival Time | Burst Time |
|---------|-------------|-----------|
| P1 | 0 | 8 |
| P2 | 1 | 4 |
| P3 | 2 | 9 |
| P4 | 3 | 5 |

**Time 0**: Only P1 has arrived -> P1 starts executing (remaining: 8)

**Time 1**: P2 arrives (burst=4) vs P1 (remaining=7) -> 4 < 7 -> **Preempt P1, run P2**

**Time 2**: P3 arrives (burst=9) vs P2 (remaining=3) -> 9 > 3 -> **P2 continues**

**Time 3**: P4 arrives (burst=5) vs P2 (remaining=2) -> 5 > 2 -> **P2 continues**

**Time 5**: P2 completes -> ready: P1(remaining=7), P3(9), P4(5) -> **Select P4** (5 is minimum)

**Time 10**: P4 completes -> ready: P1(remaining=7), P3(9) -> **Select P1**

**Time 17**: P1 completes -> ready: P3(9) -> **Select P3**

**Time 26**: P3 completes -> All processes finished

> **Exam Tip:** When solving SRTF problems, always check at each new arrival whether preemption occurs. The comparison is always between the new process's *total* burst and the current process's *remaining* burst. At non-arrival times (when no new process arrives), no comparison is needed — the current process continues.

### 5.3 SRTF — Gantt Chart and Calculation

```text
Gantt Chart:
+----+----------+------------+-----------------+--------------------+
| P1 |    P2    |     P4     |       P1        |         P3         |
+----+----------+------------+-----------------+--------------------+
0    1          5            10                17                   26
```

| Process | Arrival | Burst | Completion | Turnaround | Waiting |
|---------|---------|-------|-----------|------------|---------|
| P1 | 0 | 8 | 17 | 17-0 = 17 | 17-8 = **9** |
| P2 | 1 | 4 | 5 | 5-1 = 4 | 4-4 = **0** |
| P3 | 2 | 9 | 26 | 26-2 = 24 | 24-9 = **15** |
| P4 | 3 | 5 | 10 | 10-3 = 7 | 7-5 = **2** |

```text
Average Waiting Time    = (9 + 0 + 15 + 2) / 4 = 26/4 = 6.5 ms
Average Turnaround Time = (17 + 4 + 24 + 7) / 4 = 52/4 = 13.0 ms
```

### 5.4 SJF vs SRTF — Same Data Comparison

Comparing Nonpreemptive SJF with the same data (P1:0,8 / P2:1,4 / P3:2,9 / P4:3,5):

```text
Nonpreemptive SJF:
+------------------+----------+------------+--------------------+
|        P1        |    P2    |     P4     |         P3         |
+------------------+----------+------------+--------------------+
0                  8         12           17                   26
```

| Algorithm | P1 wait | P2 wait | P3 wait | P4 wait | **Average** |
|-----------|---------|---------|---------|---------|-------------|
| **SJF** | 0 | 7 | 15 | 9 | **7.75** |
| **SRTF** | 9 | 0 | 15 | 2 | **6.5** |

- SRTF is **superior** in average waiting time (6.5 < 7.75)
- Preemption allows shorter processes to be handled faster

---

<br>

## 6. Round-Robin Scheduling

### 6.1 Round-Robin (RR)

Each process is allocated a **fixed time quantum (q)** and executed in rotation:

1. Manage the ready queue as a **circular queue**
2. Each process uses the CPU for at most **q time units**
3. If still running after q time -> **preempt**, move to the **end** of the ready queue
4. If the CPU burst is shorter than q -> voluntarily release the CPU

- **Preemptive** scheduling
- Essentially FCFS with **preemption** added

> Round-Robin is the scheduling equivalent of "taking turns." Just as students might take turns presenting for 5 minutes each, processes take turns using the CPU for q time units each. No process can hog the CPU, and everyone gets a fair share. This makes it the natural choice for interactive systems like desktops and servers.

### 6.2 RR — Fairness Guarantee

With n processes and time quantum = q:

```text
Each process receives 1/n of the CPU time (in chunks of at most q)

Maximum wait time: (n - 1) * q

Example: n = 5, q = 20ms
-> Each process gets up to 20ms of CPU every 100ms
-> Waits at most 80ms before getting the CPU again
```

- No process waits more than **(n-1) * q**
- Favorable for guaranteeing **response time**
- **Fair** CPU allocation to all processes

### 6.3 RR — Example (q = 4ms)

All processes arrive at time 0, time quantum = 4ms:

| Process | Burst Time |
|---------|-----------|
| P1 | 24 ms |
| P2 | 3 ms |
| P3 | 3 ms |

```text
Gantt Chart:
+------+----+----+------+------+------+------+------+------+
|  P1  | P2 | P3 |  P1  |  P1  |  P1  |  P1  |  P1  |  P1  |
+------+----+----+------+------+------+------+------+------+
0      4    7    10     14     18     22     26     30
```

- P1: Runs 4ms then preempted -> P2: 3ms (< q) voluntarily returns -> P3: 3ms returns -> P1 repeats

| Process | Execution Intervals | Waiting Time | Turnaround |
|---------|---------------------|-------------|-----------|
| P1 | 0-4, 10-14, 14-18, 18-22, 22-26, 26-30 | 10-4 = **6** | **30** |
| P2 | 4-7 | **4** | **7** |
| P3 | 7-10 | **7** | **10** |

```text
Average Waiting Time = (6 + 4 + 7) / 3 = 17/3 = 5.67 ms
```

- FCFS (order P1,P2,P3): average waiting = **17 ms**
- RR (q=4): average waiting = **5.67 ms** -> significant improvement!

### 6.4 RR — Time Quantum Too Large or Too Small

**If q is very large** (larger than all bursts):

```text
When q = 100 (all bursts < 100):
+----------------------------+-----+-----+
|            P1(24)          |P2(3)|P3(3)|
+----------------------------+-----+-----+
0                            24    27    30

-> Behaves identically to FCFS!
-> Preemption never occurs
```

- If the time quantum is **larger than all CPU bursts**, RR = FCFS
- The response time improvement effect disappears

**If q is very small** (e.g., q = 1ms):

*Silberschatz, Figure 5.5 — How a smaller time quantum increases context switches*

- Context switch count **increases dramatically** -> overhead grows
- If context switch time exceeds 10% of the quantum -> **inefficient**
- Real systems: context switch ~ **10 us**, quantum ~ **10-100 ms**

### 6.5 RR — Time Quantum and Turnaround Time

Effect of q on turnaround time (3 processes, each burst = 10):

*Silberschatz, Figure 5.6 — How turnaround time varies with the time quantum*

**Rule of Thumb**:
- Ideally, **80%** of CPU bursts should finish within the time quantum
- If q is too small: turnaround worsens due to context switch overhead
- If q is too large: behaves like FCFS, worsening response time
- Choosing an appropriate q is **critical**

> **Exam Tip:** The 80% rule is a key heuristic to remember. It means that the time quantum should be large enough that most processes complete their CPU burst within a single quantum (avoiding unnecessary preemption), but small enough that the few long-burst processes don't monopolize the CPU. Typical values in real systems are 10-100 ms.

### 6.6 RR — Detailed Example (4 Processes)

q = 3ms, all processes arrive at time 0:

| Process | Burst Time |
|---------|-----------|
| P1 | 8 ms |
| P2 | 5 ms |
| P3 | 3 ms |
| P4 | 6 ms |

```text
Gantt Chart (q = 3):
+-----+-----+-----+-----+-----+-----+-----+----+
| P1  | P2  | P3  | P4  | P1  | P2  | P4  | P1 |
+-----+-----+-----+-----+-----+-----+-----+----+
0     3     6     9     12    15    17    20   22
```

- P3 has burst=3=q -> completes in one turn
- P2 has burst=5 -> completes in two turns (3 + 2)
- P4 has burst=6 -> completes in two turns (3 + 3)
- P1 has burst=8 -> completes in three turns (3 + 3 + 2)

| Process | Burst | Completion | Turnaround | Waiting |
|---------|-------|-----------|------------|---------|
| P1 | 8 | 22 | 22 | **14** |
| P2 | 5 | 17 | 17 | **12** |
| P3 | 3 | 9 | 9 | **6** |
| P4 | 6 | 20 | 20 | **14** |

```text
Average Waiting Time    = (14 + 12 + 6 + 14) / 4 = 46/4 = 11.5 ms
Average Turnaround Time = (22 + 17 + 9 + 20) / 4 = 68/4 = 17.0 ms
```

- RR has a longer average turnaround than SJF but **superior response time**
- All processes use the CPU relatively **fairly**

---

<br>

## 7. Algorithm Comparison

### 7.1 Four Algorithms on the Same Data

Data: P1(0,24), P2(0,3), P3(0,3) — all arrive at time 0

| Algorithm | P1 wait | P2 wait | P3 wait | **Avg WT** | Note |
|-----------|---------|---------|---------|-----------|------|
| FCFS (P1->P2->P3) | 0 | 24 | 27 | **17.0** | Worst case |
| FCFS (P2->P3->P1) | 6 | 0 | 3 | **3.0** | Order-dependent |
| SJF | 6 | 0 | 3 | **3.0** | Optimal |
| RR (q=4) | 6 | 4 | 7 | **5.67** | Fairness |

- **SJF** is optimal in average waiting time
- **RR** is advantageous in terms of response time
- **FCFS** shows high performance variability depending on order

### 7.2 Comprehensive Characteristics Comparison

| Characteristic | FCFS | SJF | SRTF | RR |
|----------------|------|-----|------|-----|
| Preemption | No | No | **Yes** | **Yes** |
| Optimal WT | No | **Yes** | **Yes** (preemptive) | No |
| Starvation* | No | Possible | Possible | **No** |
| Response Time | Poor | Moderate | Moderate | **Excellent** |
| Implementation complexity | Very simple | Burst prediction needed | Burst prediction needed | Timer needed |
| Convoy Effect | **Occurs** | None | None | None |

**Starvation** means a process waits indefinitely because higher-priority or shorter processes keep arriving and always get scheduled first.

> This table is a high-value exam reference. Note the trade-offs: SJF/SRTF are optimal for waiting time but can cause starvation (long processes may wait indefinitely if short processes keep arriving). RR prevents starvation and has excellent response time but has higher average waiting time. FCFS is simple but has the worst overall characteristics. These trade-offs motivate the more advanced algorithms in Week 7 (priority scheduling, multilevel queues).

---

<br>

## 8. Lab — Scheduling Algorithm Simulator

### 8.1 Lab: Data Structure

```python
class Process:
    def __init__(self, pid, arrival, burst):
        self.pid = pid
        self.arrival = arrival
        self.burst = burst
        self.remaining = burst    # Used in SRTF, RR
        self.completion = 0
        self.waiting = 0
        self.turnaround = 0
        self.first_run = -1       # For response time calculation

def print_gantt(timeline):
    """timeline: [(pid, start, end), ...]"""
    header = "|"
    footer = ""
    for pid, start, end in timeline:
        width = max(len(f" P{pid} "), end - start)
        header += f" P{pid} ".center(width) + "|"
    print(header)
    # Time display
    print("0", end="")
    for pid, start, end in timeline:
        print(f"{'':>{end - start + 2}}{end}", end="")
    print()
```

### 8.2 Lab: FCFS Implementation

```python
def fcfs(processes):
    """First-Come, First-Served Scheduling"""
    # Sort by arrival order
    procs = sorted(processes, key=lambda p: p.arrival)
    time = 0
    timeline = []

    for p in procs:
        if time < p.arrival:
            time = p.arrival  # CPU idle

        start = time
        time += p.burst
        p.completion = time
        p.turnaround = p.completion - p.arrival
        p.waiting = p.turnaround - p.burst
        timeline.append((p.pid, start, time))

    return timeline

# Test
procs = [Process(1,0,24), Process(2,0,3), Process(3,0,3)]
fcfs(procs)
# -> Average Waiting Time = 17.0
```

### 8.3 Lab: SJF Implementation (Nonpreemptive)

```python
def sjf(processes):
    """Shortest-Job-First (Nonpreemptive)"""
    procs = [p for p in processes]  # Copy
    n = len(procs)
    completed = 0
    time = 0
    done = [False] * n
    timeline = []

    while completed < n:
        # Select the arrived process with the shortest burst
        candidates = [(i, procs[i]) for i in range(n)
                      if not done[i] and procs[i].arrival <= time]

        if not candidates:
            time += 1
            continue

        idx, p = min(candidates, key=lambda x: x[1].burst)
        start = time
        time += p.burst
        p.completion = time
        p.turnaround = p.completion - p.arrival
        p.waiting = p.turnaround - p.burst
        done[idx] = True
        completed += 1
        timeline.append((p.pid, start, time))

    return timeline
```

### 8.4 Lab: RR Implementation

```python
from collections import deque

def round_robin(processes, quantum):
    """Round-Robin Scheduling"""
    procs = sorted(processes, key=lambda p: p.arrival)
    for p in procs:
        p.remaining = p.burst

    queue = deque()  # deque = double-ended queue; popleft() removes from front in O(1), efficient for FIFO ready queue
    time = 0
    idx = 0
    timeline = []

    # Add processes arriving at time 0
    while idx < len(procs) and procs[idx].arrival <= time:
        queue.append(procs[idx])
        idx += 1

    while queue:
        p = queue.popleft()
        run_time = min(quantum, p.remaining)
        start = time
        time += run_time
        p.remaining -= run_time

        # Add processes that arrived during this interval
        while idx < len(procs) and procs[idx].arrival <= time:
            queue.append(procs[idx])
            idx += 1

        if p.remaining > 0:
            queue.append(p)  # Not finished -> move to back
        else:
            p.completion = time
            p.turnaround = p.completion - p.arrival
            p.waiting = p.turnaround - p.burst

        timeline.append((p.pid, start, time))

    return timeline
```

### 8.5 Lab: Testing and Comparing Results

```python
# Test data
data = [(1, 0, 10), (2, 1, 5), (3, 2, 8), (4, 3, 6)]

print("=== FCFS ===")
procs = [Process(*d) for d in data]
fcfs(procs)
for p in procs:
    print(f"P{p.pid}: WT={p.waiting}, TT={p.turnaround}")

print("\n=== SJF ===")
procs = [Process(*d) for d in data]
sjf(procs)
for p in procs:
    print(f"P{p.pid}: WT={p.waiting}, TT={p.turnaround}")

print("\n=== RR (q=4) ===")
procs = [Process(*d) for d in data]
round_robin(procs, 4)
for p in procs:
    print(f"P{p.pid}: WT={p.waiting}, TT={p.turnaround}")
```

Test data: P1(0,10), P2(1,5), P3(2,8), P4(3,6) — compare results across all 3 algorithms.

> **Comparison with previous labs**: In Weeks 4-5, we used threads to parallelize computation. Here, we're implementing the OS's own scheduling logic. Notice how the choice of algorithm dramatically changes process behavior — the same set of processes gets very different waiting and turnaround times depending on which algorithm is used. This is why CPU scheduling is considered one of the most important OS design decisions.

---

<br>

## Summary

| Concept | Key Summary |
|:--------|:-----------|
| CPU-I/O Burst | Processes **alternate** between CPU bursts and I/O bursts |
| CPU Scheduler | **Selects** the next process from the ready queue (Short-term Scheduler) |
| Preemptive vs Nonpreemptive | Most modern OS use **preemptive** (all 4 circumstances) |
| Dispatcher | Performs context switch, switches to user mode, jumps to PC |
| Scheduling Criteria | CPU Util, Throughput **maximize** / TT, WT, RT **minimize** |
| FCFS | Simple but causes **Convoy Effect** |
| SJF | Optimal but burst **prediction impossible** (Exponential Averaging) |
| SRTF | **Preemptive version** of SJF |
| RR | **Fair** CPU distribution via time quantum, good response time |
| Textbook Scope | Silberschatz Ch 5, Sections 5.1–5.3 |

---


<br>

## Self-Check Questions

1. What is the CPU-I/O burst cycle? Explain how it forms the basis for CPU scheduling.
2. What are the four circumstances under which CPU scheduling decisions are made? Which of these are present only in preemptive scheduling?
3. Distinguish between preemptive and nonpreemptive scheduling. Why do all modern operating systems use preemptive scheduling?
4. What is the difference between the CPU Scheduler and the Dispatcher? What are the three functions of the Dispatcher?
5. List and explain the five scheduling criteria. Which should be maximized and which minimized?
6. Given three processes with burst times 24, 3, and 3 (all arriving at time 0), calculate the average waiting time under FCFS and SJF. What is the convoy effect?
7. Why is SJF optimal? If it is optimal, why isn't it used in practice? Explain exponential averaging and how it addresses this limitation.
8. Given processes P1(0,8), P2(1,4), P3(2,9), P4(3,5), draw the Gantt chart and compute average waiting time for both SJF and SRTF.
9. How does Round-Robin scheduling guarantee fairness? What is the maximum waiting time for a process with n processes and quantum q?
10. What happens when the time quantum in Round-Robin is very large? Very small? What is the rule of thumb for choosing an appropriate quantum?

---
