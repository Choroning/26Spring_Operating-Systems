# Week 6 Lecture — CPU Scheduling (1)

[Repository Guide](../../README.md) | [English](Concepts_Lecture.md) | [한국어](Concepts_Lecture.ko.md)

> **Last Updated:** 2026-10-20
>
> Operating System Concepts, Silberschatz, Galvin, and Gagne, Ch 5 (5.1-5.3)

> **Learning Objectives:**
> 1. Explain the CPU-I/O burst cycle and the need for scheduling
> 2. Distinguish between preemptive and nonpreemptive scheduling
> 3. Explain the role of the dispatcher and dispatch latency
> 4. List and explain the five scheduling criteria
> 5. Apply the FCFS, SJF, SRTF, and RR algorithms and draw their Gantt charts
> 6. Predict CPU bursts using exponential averaging

---

## Table of Contents

- [1. Basic Concepts](#1-basic-concepts)
  - [1.1 Multiprogramming and CPU Scheduling](#11-multiprogramming-and-cpu-scheduling)
  - [1.2 CPU-I/O Burst Cycle](#12-cpu-io-burst-cycle)
  - [1.3 CPU Burst Distribution](#13-cpu-burst-distribution)
  - [1.4 I/O-bound vs CPU-bound Process](#14-io-bound-vs-cpu-bound-process)
  - [1.5 CPU Scheduler (Short-term Scheduler)](#15-cpu-scheduler-short-term-scheduler)
  - [1.6 When Scheduling Occurs: 4 Circumstances](#16-when-scheduling-occurs-4-circumstances)
  - [1.7 Nonpreemptive vs Preemptive Scheduling](#17-nonpreemptive-vs-preemptive-scheduling)
  - [1.8 Issues with Preemptive Scheduling](#18-issues-with-preemptive-scheduling)
  - [1.9 Dispatcher](#19-dispatcher)
  - [1.10 Dispatch Latency](#110-dispatch-latency)
- [2. Scheduling Criteria](#2-scheduling-criteria)
  - [2.1 Scheduling Criteria: Overview](#21-scheduling-criteria-overview)
  - [2.2 CPU Utilization and Throughput](#22-cpu-utilization-and-throughput)
  - [2.3 Turnaround Time and Waiting Time](#23-turnaround-time-and-waiting-time)
  - [2.4 Response Time](#24-response-time)
  - [2.5 Scheduling Criteria: Summary](#25-scheduling-criteria-summary)
- [3. FCFS Scheduling](#3-fcfs-scheduling)
  - [3.1 FCFS (First-Come, First-Served)](#31-fcfs-first-come-first-served)
  - [3.2 FCFS: Example 1 (Arrival Order: P1, P2, P3)](#32-fcfs-example-1-arrival-order-p1-p2-p3)
  - [3.3 FCFS: Example 2 (Arrival Order: P2, P3, P1)](#33-fcfs-example-2-arrival-order-p2-p3-p1)
  - [3.4 FCFS: Convoy Effect](#34-fcfs-convoy-effect)
  - [3.5 FCFS: Characteristics Summary](#35-fcfs-characteristics-summary)
- [4. SJF Scheduling](#4-sjf-scheduling)
  - [4.1 SJF (Shortest-Job-First)](#41-sjf-shortest-job-first)
  - [4.2 SJF: Proof of Optimality](#42-sjf-proof-of-optimality)
  - [4.3 SJF: Example (Nonpreemptive)](#43-sjf-example-nonpreemptive)
  - [4.4 Predicting the Next Burst Length](#44-predicting-the-next-burst-length)
  - [4.5 Exponential Averaging: Formula](#45-exponential-averaging-formula)
  - [4.6 Exponential Averaging: Expansion](#46-exponential-averaging-expansion)
  - [4.7 Exponential Averaging: Calculation Example](#47-exponential-averaging-calculation-example)
  - [4.8 Exponential Averaging: Comparing Alpha Values](#48-exponential-averaging-comparing-alpha-values)
- [5. SRTF: Preemptive SJF](#5-srtf-preemptive-sjf)
  - [5.1 SRTF (Shortest-Remaining-Time-First)](#51-srtf-shortest-remaining-time-first)
  - [5.2 SRTF: Step-by-Step Example](#52-srtf-step-by-step-example)
  - [5.3 SRTF: Gantt Chart and Calculation](#53-srtf-gantt-chart-and-calculation)
  - [5.4 SJF vs SRTF: Same Data Comparison](#54-sjf-vs-srtf-same-data-comparison)
- [6. Round-Robin Scheduling](#6-round-robin-scheduling)
  - [6.1 Round-Robin (RR)](#61-round-robin-rr)
  - [6.2 RR: Fairness Guarantee](#62-rr-fairness-guarantee)
  - [6.3 RR: Example (q = 4ms)](#63-rr-example-q--4ms)
  - [6.4 RR: Time Quantum Too Large or Too Small](#64-rr-time-quantum-too-large-or-too-small)
  - [6.5 RR: Time Quantum and Turnaround Time](#65-rr-time-quantum-and-turnaround-time)
  - [6.6 RR: Detailed Example (4 Processes)](#66-rr-detailed-example-4-processes)
- [7. Algorithm Comparison](#7-algorithm-comparison)
  - [7.1 Three Algorithms on the Same Data](#71-three-algorithms-on-the-same-data)
  - [7.2 Comprehensive Characteristics Comparison](#72-comprehensive-characteristics-comparison)
- [8. Lab: Scheduling Algorithm Simulator](#8-lab-scheduling-algorithm-simulator)
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

The key idea of CPU scheduling is to keep the CPU busy at all times to **maximize utilization**.

- In a single CPU core system, only one process can run at a time.
- While a process waits for I/O, the CPU is **idle** and its time is wasted.
- **Multiprogramming** keeps multiple processes in memory; when one waits, the CPU is allocated to another process.
- CPU scheduling is one of the **most fundamental functions** of an operating system.

> **Note:** **Analogy.** CPU scheduling can be compared to a restaurant kitchen with one stove (CPU). If a chef puts a dish in the oven (I/O burst) and only stands there waiting, time is wasted. A good kitchen manager (scheduler) assigns the chef to start preparing another dish (CPU burst for another process) while the first one bakes. Multiprogramming ensures the stove is always in use.

### 1.2 CPU-I/O Burst Cycle

Process execution consists of a cycle in which **CPU bursts** and **I/O bursts** alternate (Figure 1, textbook Figure 5.1).

![Figure 1. Alternating sequence of CPU and I/O bursts](../../images/Week06_p02.png)

*Figure 1. Alternating sequence of CPU and I/O bursts*

- A **CPU burst** is the interval during which the CPU executes instructions.
- An **I/O burst** is the interval during which the process waits for I/O completion.
- In the final CPU burst, the process requests **termination via a system call**.

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

- All processes follow this pattern, but the **burst length and frequency** differ.
- This difference is the key basis for choosing scheduling algorithms.

> **Key Point:** The CPU-I/O burst cycle is the fundamental rhythm of process execution. Every process alternates between thinking (CPU burst) and waiting (I/O burst). The scheduling algorithm's job is to fill the gaps efficiently. When one process enters an I/O burst, another process should be ready to start its CPU burst.

### 1.3 CPU Burst Distribution

The distribution of CPU burst lengths follows an **exponential or hyperexponential** decay pattern (Figure 2, textbook Figure 5.2). A hyperexponential distribution varies more than an exponential distribution with the same mean: bursts cluster even more strongly at the short end, yet very long bursts still appear occasionally (a heavier tail).

- **Short CPU bursts** are very frequent, so most bursts are short.
- **Long CPU bursts** occur rarely.

![Figure 2. Histogram of CPU-burst durations](../../images/Week06_p03.png)

*Figure 2. Histogram of CPU-burst durations*

> **Note:** This distribution matters because it justifies the design of algorithms like SJF. Since most bursts are short, prioritizing short bursts means most processes get served quickly, and only the rare long-burst processes wait longer.

### 1.4 I/O-bound vs CPU-bound Process

Processes are classified as follows based on the CPU burst distribution.

| Category | I/O-bound Process | CPU-bound Process |
|----------|-------------------|-------------------|
| CPU burst | Many **short** bursts | Few **long** bursts |
| I/O burst | Frequent I/O requests | Rare I/O requests |
| Examples | Web server, database | Scientific computation, video encoding |
| Scheduling needs | Fast response time | Long CPU allocation time |

*Table 1. Comparison of I/O-bound and CPU-bound processes*

- **Scheduling algorithms** must handle both types efficiently.
- This distribution characteristic forms the theoretical basis for algorithms like SJF and RR.

> **Key Point:** Classifying processes as I/O-bound or CPU-bound explains why different scheduling algorithms favor different workloads. FCFS favors CPU-bound processes (they get long uninterrupted runs), while RR favors I/O-bound processes (they get quick, frequent access to the CPU).

### 1.5 CPU Scheduler (Short-term Scheduler)

When the CPU becomes idle, the next process to run is selected from the **ready queue**.

```text
                    +-------------+
  new process --->  | Ready Queue | ---> CPU Scheduler ---> CPU
                    +-------------+   selection algorithm
                          ^
                          |
                  I/O completion, etc.
```

*Figure 3. Flow of processes from the ready queue through the CPU scheduler to the CPU*

- The **CPU scheduler** is also called the short-term scheduler.
- The ready queue does not have to be FIFO.
  - It can be implemented as a FIFO queue, **priority queue**, tree, linked list, and so on.
- Records in the queue are typically **PCBs (Process Control Blocks)**.
  - The PCB (Process Control Block) covered in Week 2 stores all state information for a process (registers, program counter, priority, etc.) so that it can be resumed exactly where it left off after a context switch.
- The internal order of the queue is determined by the scheduling algorithm.

> **Key Point:** Despite its name, the ready queue is not necessarily a queue in the data structure sense. It is a collection of processes that are ready to run, and the scheduling algorithm determines how that collection is ordered. FCFS uses a FIFO queue, and SJF uses a priority queue sorted by burst length. Calling it a queue is a historical convention.

### 1.6 When Scheduling Occurs: 4 Circumstances

The **4 circumstances** in which CPU scheduling decisions are needed are as follows.

| No. | State Transition | Example | Choice |
|-----|------------------|---------|--------|
| **1** | Running -> Waiting | I/O request, wait() call | No choice |
| **2** | Running -> Ready | Interrupt occurs (timer, etc.) | **Choice exists** |
| **3** | Waiting -> Ready | I/O completion | **Choice exists** |
| **4** | Terminates | Process terminates | No choice |

*Table 2. Four circumstances in which CPU scheduling decisions occur*

- In circumstances **1 and 4**, a new process must be selected, so there is no choice about whether to schedule (no choice).
- In circumstances **2 and 3**, whether to schedule depends on the scheduling scheme (choice exists).

> **Key Point:** Understanding these four circumstances is fundamental to understanding the difference between preemptive and nonpreemptive scheduling. Circumstances 1 and 4 are mandatory scheduling points because the running process can no longer use the CPU. Circumstances 2 and 3 are optional and trigger scheduling decisions only in preemptive systems.

### 1.7 Nonpreemptive vs Preemptive Scheduling

| Category | Nonpreemptive | Preemptive |
|----------|---------------|------------|
| Scheduling points | Only at **1, 4** | At **1, 2, 3, 4** (all) |
| CPU release | Process **voluntarily** releases | OS can **forcibly** take away |
| Also known as | Cooperative scheduling | - |
| Usage | Early OS | **Most modern OS** |

*Table 3. Comparison of nonpreemptive and preemptive scheduling*

- In **nonpreemptive** scheduling, the process voluntarily releases the CPU (termination or I/O wait).
- In **preemptive** scheduling, the OS can forcibly reclaim the CPU via a timer interrupt or similar mechanism.
- Windows, macOS, Linux, and UNIX all use **preemptive** scheduling.

> **Key Point:** The criterion that separates the two schemes is that nonpreemptive scheduling acts only in circumstances 1 and 4, while preemptive scheduling acts in all four. Nonpreemptive scheduling is also called cooperative scheduling because the process cooperates by voluntarily giving up the CPU.

### 1.8 Issues with Preemptive Scheduling

Preemptive scheduling is powerful but has the following **caveats**.

**1. Race Condition**
- If a process is preempted while modifying shared data, another process may read inconsistent data.
- **Synchronization mechanisms** (mutex, semaphore, etc.) are therefore needed (covered in detail in Week 9).

A **race condition** occurs when two processes read and write shared data at the same time, and it can leave the data in an invalid state. A **mutex** (mutual exclusion lock) prevents this by allowing only one process at a time into a **critical section**, the portion of code that accesses shared data.

**2. Preemption in Kernel Mode**
- If a process is preempted while a system call is modifying kernel data structures, the kernel can become inconsistent.
- A **nonpreemptive kernel** defers the context switch until the system call completes (simple but unsuitable for real-time processing).
- A **preemptive kernel** protects kernel data with mutex locks and similar tools (most modern OS).

**3. Interrupt Handling**
- Interrupts can occur at any time, so interrupts are disabled in critical sections.

> **Note:** These issues lead into the synchronization topics of Week 9. The key point is that preemption introduces concurrency problems. If the OS can stop a process at any point, shared data might be in an inconsistent state when another process reads it. This is the fundamental reason why synchronization mechanisms exist.

### 1.9 Dispatcher

The **dispatcher** is the module that actually hands over CPU control to the process selected by the scheduler.

The dispatcher has the following three roles (Figure 4, textbook Figure 5.3).
1. It performs the **context switch**. It saves the state of the current process and restores the state of the new process.
2. It switches to **user mode**.
3. It jumps to the appropriate location of the new process. This location is given by the PC (Program Counter, the register holding the address of the next instruction).

![Figure 4. Role of the dispatcher](../../images/Week06_p05.png)

*Figure 4. Role of the dispatcher*

> **Note:** **Scheduler vs Dispatcher.** The scheduler **decides** which process runs next; the dispatcher **executes** that decision. The scheduler can be thought of as a manager who picks which employee works next, and the dispatcher as the HR person who handles the actual paperwork and office reassignment. The distinction matters because they are separate components with different responsibilities.

### 1.10 Dispatch Latency

**Dispatch latency** is the time it takes for the dispatcher to stop one process and start running another.

- It occurs on every context switch, so it should be **as short as possible**.

On Linux, the following commands show context switch counts.

```bash
# System-wide context switch count (1-second interval, 3 times)
vmstat 1 3

# Context switch count for a specific process
cat /proc/<PID>/status | grep ctxt
# voluntary_ctxt_switches:    150   (voluntary: I/O wait, etc.)
# nonvoluntary_ctxt_switches: 8     (involuntary: time slice expired, etc.)
```

- A **voluntary context switch** occurs when a process gives up the CPU by itself because it needs a resource that is currently unavailable (such as waiting for I/O).
- A **nonvoluntary context switch** occurs when the OS forces a switch, for example because the time slice expired.

> **Note:** On modern systems, a context switch typically takes about 1-10 microseconds. While this seems tiny, it adds up. A busy server performing thousands of context switches per second can lose significant CPU time to dispatch latency. This is why the time quantum in Round-Robin scheduling must be chosen carefully. Too small a quantum means too many context switches.

---

<br>

## 2. Scheduling Criteria

### 2.1 Scheduling Criteria: Overview

The five criteria for **comparing** and **evaluating** scheduling algorithms are as follows.

| Criterion | Description | Optimization Direction |
|-----------|-------------|----------------------|
| **CPU Utilization** | CPU usage rate (%) | **Maximize** |
| **Throughput** | Number of processes completed per unit time | **Maximize** |
| **Turnaround Time** | Total time from submission to completion | **Minimize** |
| **Waiting Time** | Total time spent waiting in the ready queue | **Minimize** |
| **Response Time** | Time from submission to first response | **Minimize** |

*Table 4. Five scheduling criteria and their optimization directions*

### 2.2 CPU Utilization and Throughput

**CPU Utilization**
- It is the proportion of time the CPU is performing useful work.
- Its theoretical range is 0% to 100%.
- Real systems run at around **40%** under light load and around **90%** under heavy load.
- It can be checked with the `top` command on Linux.

**Throughput**
- It is the number of processes completed per unit time.
- Long processes may complete at a rate of one every few seconds.
- Short transactions may complete at a rate of dozens per second.

- The goal is to **maximize** both criteria.
- The two criteria can be in a trade-off relationship (context switch overhead).

### 2.3 Turnaround Time and Waiting Time

**Turnaround Time**
- It is the total time from process **submission** to **completion**.
- It equals the sum of ready queue wait, CPU execution, and I/O wait time.

```text
Turnaround Time = Completion Time - Arrival Time
```

**Waiting Time**
- It is the total time spent **waiting** in the ready queue.
- It is the only time that the scheduling algorithm **directly affects**.
- CPU execution time and I/O time are independent of the algorithm.

```text
Waiting Time = Turnaround Time - Burst Time
             (for a single burst, with no I/O)
```

> **Key Point:** Waiting time is the most commonly used metric for comparing scheduling algorithms because it isolates the effect of the scheduling algorithm. Turnaround time includes CPU execution and I/O time, which are the same regardless of which algorithm is used. Waiting time captures *only* the delay introduced by the scheduling decision.

### 2.4 Response Time

**Response Time**
- It is the time from process **submission** to the moment the **first response is produced**.
- It is the most important criterion in interactive systems.

```text
Response Time = First Run Time - Arrival Time
```

Turnaround time and response time compare as follows.

| Comparison | Turnaround Time | Response Time |
|------------|-----------------|---------------|
| Measured interval | Submission to **completion** | Submission to **first response** |
| Important for | Batch system | **Interactive** system |
| Example | 30 seconds to complete the job | 0.5 seconds to first output |

*Table 5. Comparison of turnaround time and response time*

- In interactive systems, **minimizing the variance** of response time is important.
- **Predictable** response time is more favorable for user satisfaction than a lower average response time.

### 2.5 Scheduling Criteria: Summary

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

*Figure 5. Criteria to maximize and criteria to minimize*

- In most cases, the **average** is optimized.
- In some cases, the **minimum** or **maximum** is optimized (e.g., minimizing the maximum response time).

---

<br>

## 3. FCFS Scheduling

### 3.1 FCFS (First-Come, First-Served)

FCFS (First-Come, First-Served) is the **simplest** CPU scheduling algorithm and executes processes in **arrival order**.

- It is implemented with a **FIFO queue**.
- When a process arrives at the ready queue, it is added to the **tail**.
- When the CPU is free, it is allocated to the process at the **head**.
- It is a **nonpreemptive** scheduling algorithm.

```text
Ready Queue (FIFO):

  head ---> [ P1 ] ---> [ P2 ] ---> [ P3 ] <--- tail
              |
              v
            CPU allocation
```

*Figure 6. FIFO ready queue that allocates the CPU to the process at the head*

### 3.2 FCFS: Example 1 (Arrival Order: P1, P2, P3)

The processes arrive in the order P1, P2, P3, all at time 0.

| Process | Burst Time |
|---------|-----------|
| P1 | 24 ms |
| P2 | 3 ms |
| P3 | 3 ms |

*Table 6. Processes and burst times for the FCFS example*

A **Gantt chart** in scheduling is a horizontal bar diagram where each bar shows which process is using the CPU during a given time interval. The numbers along the bottom are timestamps.

```text
Gantt Chart:
+-------------------------+----+----+
|            P1           | P2 | P3 |
+-------------------------+----+----+
0                         24   27   30
```

*Figure 7. FCFS Gantt chart for arrival order P1, P2, P3*

| Process | Waiting Time | Turnaround Time |
|---------|-------------|----------------|
| P1 | 0 | 24 |
| P2 | 24 | 27 |
| P3 | 27 | 30 |
| **Average** | **(0+24+27)/3 = 17** | **(24+27+30)/3 = 27** |

*Table 7. Waiting and turnaround times for arrival order P1, P2, P3*

### 3.3 FCFS: Example 2 (Arrival Order: P2, P3, P1)

The same processes arriving in the order P2, P3, P1 give the following result.

```text
Gantt Chart:
+----+----+-------------------------+
| P2 | P3 |            P1           |
+----+----+-------------------------+
0    3    6                         30
```

*Figure 8. FCFS Gantt chart for arrival order P2, P3, P1*

| Process | Waiting Time | Turnaround Time |
|---------|-------------|----------------|
| P1 | 6 | 30 |
| P2 | 0 | 3 |
| P3 | 3 | 6 |
| **Average** | **(6+0+3)/3 = 3** | **(30+3+6)/3 = 13** |

*Table 8. Waiting and turnaround times for arrival order P2, P3, P1*

- Changing only the order reduces the average waiting time dramatically, **from 17 to 3**.
- FCFS performance **varies significantly** depending on arrival order.

### 3.4 FCFS: Convoy Effect

The **convoy effect** is a phenomenon where short processes queue up behind a single process with a long CPU burst.

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

*Figure 9. Convoy effect in which one CPU-bound process delays I/O-bound processes*

- When the CPU-bound process monopolizes the CPU, the I/O-bound processes wait in the ready queue.
- Meanwhile, the **I/O device is idle**, so the **utilization drops** for both the CPU and the I/O devices.
- It is **unsuitable for interactive systems** because a single process can monopolize the CPU for a long time.

> **Note:** **Analogy.** The convoy effect is named by analogy with a slow truck on a single-lane road: all faster cars (short processes) must wait behind it. In scheduling terms, both the CPU and the I/O devices are underutilized. The I/O device sits idle while I/O-bound processes wait for the CPU, and the CPU sits idle later when all the I/O-bound processes do their I/O at the same time.

### 3.5 FCFS: Characteristics Summary

| Characteristic | Description |
|----------------|-------------|
| Type | **Nonpreemptive** |
| Implementation | FIFO queue, very simple |
| Average Waiting Time | Generally **long** (not optimal) |
| Convoy Effect | **Occurs** (CPU-bound blocks I/O-bound) |
| Interactive suitability | **Unsuitable** (cannot distribute CPU at regular intervals) |
| Advantage | Simple to implement, easy to understand |
| Disadvantage | Performance heavily depends on arrival order |

*Table 9. Characteristics of FCFS*

---

<br>

## 4. SJF Scheduling

### 4.1 SJF (Shortest-Job-First)

SJF (Shortest-Job-First) selects the process with the **shortest next CPU burst** first.

- A more accurate name is the **Shortest-Next-CPU-Burst** algorithm.
- If burst lengths are equal, ties are broken with **FCFS**.
- Both nonpreemptive and preemptive versions exist.

**Key property**: SJF guarantees the **minimum average waiting time** for a given set of processes, so it is **optimal**.

**Fundamental problem**: The next CPU burst length **cannot be known exactly**.

### 4.2 SJF: Proof of Optimality

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

*Figure 10. Waiting times when the long job runs first versus when the short job runs first*

- Moving the **shorter process** forward **decreases** its waiting time **significantly**.
- The **longer process's** waiting time **increases slightly**.
- As a result, the overall average waiting time **decreases**.
- Generalizing this shows that SJF is **provably optimal**.

> **[Algorithms]** The proof uses the exchange argument. In any schedule that is not SJF-ordered, there is an adjacent pair where a longer job immediately precedes a shorter one. Swapping them reduces total waiting time without affecting the waiting times of other jobs. Repeating such swaps eventually yields the SJF order. This is equivalent to the sorting argument that ordering jobs by burst time minimizes total completion time.

### 4.3 SJF: Example (Nonpreemptive)

The following is a nonpreemptive SJF example in which all processes arrive at time 0.

| Process | Burst Time |
|---------|-----------|
| P1 | 6 ms |
| P2 | 8 ms |
| P3 | 7 ms |
| P4 | 3 ms |

*Table 10. Processes and burst times for the nonpreemptive SJF example*

```text
Gantt Chart (SJF order: P4 -> P1 -> P3 -> P2):
+----+--------+----------+------------+
| P4 |   P1   |    P3    |     P2     |
+----+--------+----------+------------+
0    3        9         16            24
```

*Figure 11. Gantt chart of nonpreemptive SJF*

| Process | Waiting Time |
|---------|-------------|
| P1 | 3 |
| P2 | 16 |
| P3 | 9 |
| P4 | 0 |
| **Average** | **(3+16+9+0)/4 = 7** |

*Table 11. Waiting times under nonpreemptive SJF*

If the same processes ran under FCFS (order P1, P2, P3, P4), the average waiting time would be (0+6+14+21)/4 = **10.25**.

### 4.4 Predicting the Next Burst Length

SJF is **optimal** but cannot be implemented as is at the level of CPU scheduling, because the **next CPU burst length cannot be known**.

The solution is to **predict** the next burst based on past burst history.

- Past CPU bursts tend to repeat with similar lengths.
- **Exponential averaging** is used for prediction.

### 4.5 Exponential Averaging: Formula

The idea is to form a **weighted average** between the most recent observed burst and the previous prediction, so that neither history nor the latest measurement is ignored entirely.

The formula for predicting the next CPU burst is as follows.

```text
tau(n+1) = alpha * t(n) + (1 - alpha) * tau(n)
```

| Symbol | Meaning |
|--------|---------|
| `t(n)` | The n-th **actual** CPU burst length (most recent observed value) |
| `tau(n)` | The n-th **predicted value** (previous prediction) |
| `tau(n+1)` | The next CPU burst **predicted value** |
| `alpha` | Weight (0 <= alpha <= 1) |

*Table 12. Symbols in the exponential averaging formula*

**Characteristics depending on alpha:**

| alpha value | Meaning |
|-------------|---------|
| alpha = 0 | `tau(n+1) = tau(n)`: ignores the recent observation and uses only the past prediction |
| alpha = 1 | `tau(n+1) = t(n)`: uses only the last observation and ignores the past prediction |
| alpha = 1/2 | **Balances** recent and past values (common choice) |

*Table 13. Prediction characteristics for different alpha values*

### 4.6 Exponential Averaging: Expansion

Expanding the formula by repeated substitution gives the following.

```text
tau(n+1) = alpha * t(n)
         + (1-alpha) * alpha * t(n-1)
         + (1-alpha)^2 * alpha * t(n-2)
         + ...
         + (1-alpha)^j * alpha * t(n-j)
         + ...
         + (1-alpha)^(n+1) * tau(0)
```

- If 0 < alpha < 1, then **(1 - alpha)** also lies between 0 and 1, so the weights **decrease exponentially** going further into the past.
- The most recent observed value has the **greatest influence** on the prediction.
- The name **exponential averaging** comes from the exponential decay of the weights.

> **[Probability and Statistics]** Exponential averaging applies the Exponentially Weighted Moving Average (EWMA), which is used extensively in signal processing and time series analysis, to CPU burst prediction. The parameter alpha controls the memory length of the estimator. Higher alpha means a shorter memory that responds quickly to changes, and lower alpha means a longer memory with more stable predictions. In networking, TCP uses a similar approach to estimate round-trip time.

### 4.7 Exponential Averaging: Calculation Example

Starting with alpha = 0.5 and tau(0) = 10, the predictions are as shown in Table 14, and Figure 12 (textbook Figure 5.4) plots them.

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

*Table 14. Exponential averaging with alpha = 0.5 and tau(0) = 10*

![Figure 12. Actual CPU bursts and exponential averaging predictions](../../images/Week06_p10.png)

*Figure 12. Actual CPU bursts and exponential averaging predictions*

- Even when the actual burst changes suddenly, the predicted value follows gradually.
- When the same value repeats consecutively, the prediction converges to that value.

### 4.8 Exponential Averaging: Comparing Alpha Values

For the same burst sequence (tau(0) = 10, bursts = [6, 4, 6]), varying alpha changes the predictions as follows.

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

- With **smaller alpha**, the predicted value changes **slowly** (it depends more on history).
- With **larger alpha**, the predicted value **reacts quickly** to recent values.
- In practice, values around **alpha = 0.5** are commonly used.

---

<br>

## 5. SRTF: Preemptive SJF

### 5.1 SRTF (Shortest-Remaining-Time-First)

SRTF (Shortest-Remaining-Time-First) is the **preemptive version of SJF**.

It works as follows.
- A **comparison is performed each time** a new process arrives.
- If the new process's burst is shorter than the current process's **remaining burst**, the scheduler **preempts (switches)**.
- Otherwise, the current process continues execution.

| Comparison | SJF (Nonpreemptive) | SRTF (Preemptive) |
|------------|--------------------|--------------------|
| Comparison point | Only when CPU is free | **Each time** a new process arrives |
| Comparison target | Total burst length | **Remaining** burst length |
| Preemption | None | Yes |

*Table 15. Comparison of nonpreemptive SJF and SRTF*

### 5.2 SRTF: Step-by-Step Example

| Process | Arrival Time | Burst Time |
|---------|-------------|-----------|
| P1 | 0 | 8 |
| P2 | 1 | 4 |
| P3 | 2 | 9 |
| P4 | 3 | 5 |

*Table 16. Arrival and burst times for the SRTF example*

**Time 0**: Only P1 has arrived, so P1 starts executing (remaining 8).

**Time 1**: P2 arrives (burst 4). It is shorter than P1's remaining 7 (4 < 7), so the scheduler **preempts P1 and runs P2**.

**Time 2**: P3 arrives (burst 9). It is longer than P2's remaining 3 (9 > 3), so **P2 continues**.

**Time 3**: P4 arrives (burst 5). It is longer than P2's remaining 2 (5 > 2), so **P2 continues**.

**Time 5**: P2 completes. Among the ready processes P1 (remaining 7), P3 (9), and P4 (5), the shortest one, **P4, is selected**.

**Time 10**: P4 completes. Between P1 (remaining 7) and P3 (9), **P1 is selected**.

**Time 17**: P1 completes. The remaining **P3 is selected** (9).

**Time 26**: P3 completes, and all processes have finished.

> **Key Point:** An SRTF schedule is built by checking at each new arrival whether preemption occurs. At an arrival, the comparison is between the new process's *total* burst and the current process's *remaining* burst. While no new process arrives, the current process runs until it completes, and at completion the scheduler picks the ready process with the shortest remaining burst.

### 5.3 SRTF: Gantt Chart and Calculation

```text
Gantt Chart:
+----+----------+------------+-----------------+--------------------+
| P1 |    P2    |     P4     |       P1        |         P3         |
+----+----------+------------+-----------------+--------------------+
0    1          5            10                17                   26
```

*Figure 13. Gantt chart of SRTF*

| Process | Arrival | Burst | Completion | Turnaround | Waiting |
|---------|---------|-------|-----------|------------|---------|
| P1 | 0 | 8 | 17 | 17-0 = 17 | 17-8 = **9** |
| P2 | 1 | 4 | 5 | 5-1 = 4 | 4-4 = **0** |
| P3 | 2 | 9 | 26 | 26-2 = 24 | 24-9 = **15** |
| P4 | 3 | 5 | 10 | 10-3 = 7 | 7-5 = **2** |

*Table 17. Completion, turnaround, and waiting times under SRTF*

```text
Average Waiting Time    = (9 + 0 + 15 + 2) / 4 = 26/4 = 6.5 ms
Average Turnaround Time = (17 + 4 + 24 + 7) / 4 = 52/4 = 13.0 ms
```

### 5.4 SJF vs SRTF: Same Data Comparison

Scheduling the same data (P1: 0, 8 / P2: 1, 4 / P3: 2, 9 / P4: 3, 5) with nonpreemptive SJF gives the following.

```text
Nonpreemptive SJF:
+------------------+----------+------------+--------------------+
|        P1        |    P2    |     P4     |         P3         |
+------------------+----------+------------+--------------------+
0                  8         12           17                   26
```

*Figure 14. Gantt chart of nonpreemptive SJF for the same data*

| Algorithm | P1 wait | P2 wait | P3 wait | P4 wait | **Average** |
|-----------|---------|---------|---------|---------|-------------|
| **SJF** | 0 | 7 | 15 | 9 | **7.75** |
| **SRTF** | 9 | 0 | 15 | 2 | **6.5** |

*Table 18. Waiting times under nonpreemptive SJF and SRTF*

- SRTF is **superior** in average waiting time (6.5 < 7.75).
- Preemption allows shorter processes to be handled faster.

---

<br>

## 6. Round-Robin Scheduling

### 6.1 Round-Robin (RR)

Round-Robin allocates a **fixed time quantum (q)** to each process and executes the processes in rotation.

1. The ready queue is managed as a **circular queue**.
2. Each process uses the CPU for at most **q time units**.
3. If a process is still running after q time units, it is **preempted** and moved to the **end** of the ready queue.
4. If the CPU burst is shorter than q, the process voluntarily releases the CPU.

- It is a **preemptive** scheduling algorithm.
- It is essentially FCFS with **preemption** added.

> **Note:** **Analogy.** Round-Robin is the scheduling equivalent of taking turns. Just as students might take turns presenting for 5 minutes each, processes take turns using the CPU for q time units each. No process can hog the CPU, and every process gets a fair share. This makes RR the natural choice for interactive systems like desktops and servers.

### 6.2 RR: Fairness Guarantee

With n processes and a time quantum of q, the following holds.

```text
Each process receives 1/n of the CPU time (in chunks of at most q)

Maximum wait time: (n - 1) * q

Example: n = 5, q = 20ms
-> Each process gets up to 20ms of CPU every 100ms
-> Waits at most 80ms before getting the CPU again
```

- No process waits more than **(n-1) * q**.
- It is favorable for guaranteeing **response time**.
- It allocates the CPU **fairly** to all processes.

### 6.3 RR: Example (q = 4ms)

All processes arrive at time 0, and the time quantum is 4ms.

| Process | Burst Time |
|---------|-----------|
| P1 | 24 ms |
| P2 | 3 ms |
| P3 | 3 ms |

*Table 19. Processes and burst times for the RR example with q = 4ms*

```text
Gantt Chart:
+------+----+----+------+------+------+------+------+
|  P1  | P2 | P3 |  P1  |  P1  |  P1  |  P1  |  P1  |
+------+----+----+------+------+------+------+------+
0      4    7    10     14     18     22     26     30
```

*Figure 15. Gantt chart of RR with q = 4ms*

- P1 runs for 4ms and is preempted, P2 runs for 3ms (< q) and voluntarily releases the CPU, and P3 also runs for 3ms and releases it. P1 then runs repeatedly.

| Process | Execution Intervals | Waiting Time | Turnaround |
|---------|---------------------|-------------|-----------|
| P1 | 0-4, 10-14, 14-18, 18-22, 22-26, 26-30 | 10-4 = **6** | **30** |
| P2 | 4-7 | **4** | **7** |
| P3 | 7-10 | **7** | **10** |

*Table 20. Execution intervals, waiting times, and turnaround times of RR with q = 4ms*

```text
Average Waiting Time = (6 + 4 + 7) / 3 = 17/3 = 5.67 ms
```

- Under FCFS (order P1, P2, P3), the average waiting time is **17 ms**.
- Under RR (q = 4), the average waiting time is **5.67 ms**, a significant improvement.

### 6.4 RR: Time Quantum Too Large or Too Small

**If q is very large** (larger than all bursts), the result is as follows.

```text
When q = 100 (all bursts < 100):
+----------------------------+-----+-----+
|            P1(24)          |P2(3)|P3(3)|
+----------------------------+-----+-----+
0                            24    27    30

-> Behaves identically to FCFS!
-> Preemption never occurs
```

*Figure 16. Gantt chart of RR with q = 100*

- If the time quantum is **larger than all CPU bursts**, RR behaves the same as FCFS.
- The response time improvement disappears.

**If q is very small** (e.g., q = 1ms), context switches increase. As Figure 17 (textbook Figure 5.5) shows, a process with a processing time of 10 undergoes 0 context switches with a quantum of 12, 1 with a quantum of 6, and 9 with a quantum of 1.

![Figure 17. How a smaller time quantum increases context switches](../../images/Week06_p13.png)

*Figure 17. How a smaller time quantum increases context switches*

- The context switch count **increases dramatically**, so the overhead grows.
- If the context switch time is about 10% of the quantum, about 10% of the CPU time is spent on context switches, which is **inefficient**.
- In real systems, a context switch takes under **10 μs**, and the quantum is **10-100 ms**.

### 6.5 RR: Time Quantum and Turnaround Time

Figure 18 (textbook Figure 5.6) shows the effect of q on turnaround time. The graph plots the average turnaround time of four processes with bursts 6, 3, 1, and 7 for q from 1 to 7. The average turnaround time is 11.0 at q = 1 and 12.25 at q = 5, and it becomes 10.5 once q is 6 or more. With q of 7 or more, every burst finishes within one quantum; with q = 6, only the process with burst 7 runs a second time, but it finishes last, so the average is the same.

![Figure 18. Average turnaround time as a function of the time quantum](../../images/Week06_p14.png)

*Figure 18. Average turnaround time as a function of the time quantum*

The **rule of thumb** is as follows.
- Ideally, **80%** of CPU bursts should finish within the time quantum.
- If q is too small, turnaround worsens due to context switch overhead.
- If q is too large, RR behaves like FCFS, worsening response time.
- Choosing an appropriate q is therefore **critical**.

> **Key Point:** The 80% rule is the key heuristic for choosing the time quantum. The time quantum should be large enough that most processes complete their CPU burst within a single quantum (avoiding unnecessary preemption), but small enough that the few long-burst processes do not monopolize the CPU. Typical values in real systems are 10-100 ms.

### 6.6 RR: Detailed Example (4 Processes)

q = 3ms, and all processes arrive at time 0.

| Process | Burst Time |
|---------|-----------|
| P1 | 8 ms |
| P2 | 5 ms |
| P3 | 3 ms |
| P4 | 6 ms |

*Table 21. Processes and burst times for the detailed RR example with q = 3ms*

```text
Gantt Chart (q = 3):
+-----+-----+-----+-----+-----+-----+-----+----+
| P1  | P2  | P3  | P4  | P1  | P2  | P4  | P1 |
+-----+-----+-----+-----+-----+-----+-----+----+
0     3     6     9     12    15    17    20   22
```

*Figure 19. Gantt chart of RR with q = 3*

- P3 has a burst of 3, equal to q, so it completes in one turn.
- P2 has a burst of 5, so it completes in two turns (3 + 2).
- P4 has a burst of 6, so it completes in two turns (3 + 3).
- P1 has a burst of 8, so it completes in three turns (3 + 3 + 2).

| Process | Burst | Completion | Turnaround | Waiting |
|---------|-------|-----------|------------|---------|
| P1 | 8 | 22 | 22 | **14** |
| P2 | 5 | 17 | 17 | **12** |
| P3 | 3 | 9 | 9 | **6** |
| P4 | 6 | 20 | 20 | **14** |

*Table 22. Completion, turnaround, and waiting times of RR with q = 3*

```text
Average Waiting Time    = (14 + 12 + 6 + 14) / 4 = 46/4 = 11.5 ms
Average Turnaround Time = (22 + 17 + 9 + 20) / 4 = 68/4 = 17.0 ms
```

- RR has a longer average turnaround than SJF but **superior response time**.
- All processes use the CPU relatively **fairly**.

---

<br>

## 7. Algorithm Comparison

### 7.1 Three Algorithms on the Same Data

The data are P1(0,24), P2(0,3), and P3(0,3), all arriving at time 0. When all processes arrive at the same time, no later arrival can trigger preemption, so SRTF gives the same result as SJF.

| Algorithm | P1 wait | P2 wait | P3 wait | **Avg WT** | Note |
|-----------|---------|---------|---------|-----------|------|
| FCFS (P1->P2->P3) | 0 | 24 | 27 | **17.0** | Worst case |
| FCFS (P2->P3->P1) | 6 | 0 | 3 | **3.0** | Order-dependent |
| SJF | 6 | 0 | 3 | **3.0** | Optimal |
| RR (q=4) | 6 | 4 | 7 | **5.67** | Fairness |

*Table 23. Waiting times of FCFS, SJF, and RR on the same data*

- **SJF** is optimal in average waiting time.
- **RR** is advantageous in terms of response time.
- **FCFS** shows high performance variability depending on arrival order.

### 7.2 Comprehensive Characteristics Comparison

| Characteristic | FCFS | SJF | SRTF | RR |
|----------------|------|-----|------|-----|
| Preemption | No | No | **Yes** | **Yes** |
| Optimal WT | No | **Yes** | **Yes** (preemptive) | No |
| Starvation* | No | Possible | Possible | **No** |
| Response Time | Poor | Moderate | Moderate | **Excellent** |
| Implementation complexity | Very simple | Burst prediction needed | Burst prediction needed | Timer needed |
| Convoy Effect | **Occurs** | None | None | None |

*Table 24. Comprehensive comparison of FCFS, SJF, SRTF, and RR*

**Starvation** means a process waits indefinitely because higher-priority or shorter processes keep arriving and always get scheduled first.

> **Key Point:** This table shows the trade-offs among the four algorithms. SJF and SRTF are optimal for waiting time but can cause starvation (long processes may wait indefinitely if short processes keep arriving). RR prevents starvation and has excellent response time but has a higher average waiting time. FCFS is simple but has the worst overall characteristics. These trade-offs motivate the more advanced algorithms in Week 7 (priority scheduling, multilevel queues).

---

<br>

## 8. Lab: Scheduling Algorithm Simulator

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

The test data P1(0,10), P2(1,5), P3(2,8), P4(3,6) are used to compare the results of the three algorithms. Running the code gives average waiting times of 10.5 for FCFS, 10.0 for SJF, and 16.75 for RR (q = 4).

> **Note:** **Comparison with Previous Labs.** In Weeks 4-5, threads were used to parallelize computation. This section implements the OS's own scheduling logic. Notice how the choice of algorithm dramatically changes process behavior. The same set of processes gets very different waiting and turnaround times depending on which algorithm is used. This is why CPU scheduling is considered one of the most important OS design decisions.

---

<br>

## Summary

| Concept | Key Summary |
|:--------|:-----------|
| CPU-I/O Burst | Processes alternate between CPU bursts and I/O bursts. |
| CPU Scheduler | As the short-term scheduler, it selects the next process to run from the ready queue. |
| Preemptive vs Nonpreemptive | Most modern operating systems use preemptive scheduling, which can schedule in all four circumstances. |
| Dispatcher | It performs the context switch, switches to user mode, and jumps to the new process's PC. |
| Scheduling Criteria | CPU utilization and throughput are maximized, while turnaround, waiting, and response times are minimized. |
| FCFS | It is simple to implement but causes the convoy effect. |
| SJF | It is optimal for average waiting time, but the next burst is unknown and must be predicted with exponential averaging. |
| SRTF | It is the preemptive version of SJF and runs the process with the shortest remaining burst. |
| RR | It distributes the CPU fairly through a time quantum and gives good response time. |

---


<br>

## Self-Check Questions

1. **CPU-I/O burst cycle:** What is the CPU-I/O burst cycle? Explain how it forms the basis for CPU scheduling.

   <details>
   <summary>Answer</summary>

   Process execution alternates between **CPU bursts** (instruction execution) and **I/O bursts** (waiting on devices). Because the CPU should not sit idle while one process waits on I/O, the OS must decide which other process to run, and this is CPU scheduling. The observed burst distribution (many short bursts, few long ones) is the theoretical basis for algorithms like SJF.

   </details>

2. **Scheduling decision points:** What are the four circumstances under which CPU scheduling decisions are made? Which of these are present only in preemptive scheduling?

   <details>
   <summary>Answer</summary>

   The four circumstances are ① running → waiting (e.g., I/O request), ② running → ready (timer interrupt), ③ waiting → ready (I/O completion), and ④ termination. **Scheduling choices in ② and ③ exist only in preemptive systems**; under nonpreemptive scheduling, the current process keeps running in those cases.

   </details>

3. **Preemptive and nonpreemptive:** Distinguish between preemptive and nonpreemptive scheduling. Why do virtually all modern operating systems use preemptive scheduling?

   <details>
   <summary>Answer</summary>

   Under **nonpreemptive** scheduling, scheduling occurs only when a process voluntarily releases the CPU (termination or I/O wait). Under **preemptive** scheduling, the OS can forcibly reclaim the CPU, for example via a timer interrupt. Modern operating systems use preemption to ① guarantee responsiveness for interactive use, ② prevent a process stuck in an infinite loop from monopolizing the CPU, and ③ support real-time and multitasking workloads.

   </details>

4. **Scheduler and dispatcher:** What is the difference between the CPU scheduler and the dispatcher? What are the three functions of the dispatcher?

   <details>
   <summary>Answer</summary>

   The **scheduler** is the policy that *selects* which ready process runs next, and the **dispatcher** is the mechanism that actually hands the CPU to that process. The three functions of the dispatcher are ① performing the context switch, ② switching to user mode, and ③ jumping to the new process's PC to resume its execution.

   </details>

5. **Scheduling criteria:** List and explain the five scheduling criteria. Which should be maximized and which minimized?

   <details>
   <summary>Answer</summary>

   The criteria to maximize are **CPU utilization** (fraction of time the CPU does useful work) and **throughput** (processes completed per unit time). The criteria to minimize are **turnaround time** (from submission to completion), **waiting time** (time spent in the ready queue), and **response time** (from submission to first response).

   </details>

6. **FCFS and SJF calculation:** Given three processes with burst times 24, 3, and 3 (all arriving at time 0), calculate the average waiting time under FCFS and SJF. What is the convoy effect?

   <details>
   <summary>Answer</summary>

   Under **FCFS** (order P1, P2, P3), the waiting times are 0, 24, and 27, so the average is 51/3 = **17 ms**. Under **SJF** (shortest first, order P2, P3, P1), the waiting times are 6 for P1, 0 for P2, and 3 for P3, so the average is 9/3 = **3 ms**. The **convoy effect** is the phenomenon in FCFS where short I/O-bound processes pile up behind a long CPU-bound process, leaving both the CPU and the I/O devices underutilized.

   </details>

7. **SJF and exponential averaging:** Why is SJF optimal? If it is optimal, why is it not used as is in practice? Explain exponential averaging and how it addresses this limitation.

   <details>
   <summary>Answer</summary>

   SJF is **provably optimal** for average waiting time because running a short job first reduces the waiting time of every job queued behind it. It cannot be used as is because future CPU burst lengths are unknown. **Exponential averaging** predicts the next burst from history. The formula is `tau(n+1) = alpha * t(n) + (1 - alpha) * tau(n)`, where `t(n)` is the most recent burst and `alpha` (0 <= alpha <= 1) weights recency. The prediction is then used to judge which burst is short.

   </details>

8. **SJF and SRTF Gantt charts:** Given processes P1(0,8), P2(1,4), P3(2,9), P4(3,5), draw the Gantt chart and compute the average waiting time for both SJF and SRTF.

   <details>
   <summary>Answer</summary>

   - **SJF (nonpreemptive)**: P1 runs during 0-8; among the arrived P2(4), P3(9), and P4(5), P2 is picked and runs during 8-12, then P4 runs during 12-17 and P3 during 17-26. The waiting times are P1 = 0, P2 = 7, P3 = 15, P4 = 9, and the average is 31/4 = **7.75 ms**.
   - **SRTF (preemptive)**: P1 runs during 0-1 (remaining 7). P2 arrives at time 1 (4 < 7), preempts P1, and runs during 1-5. At time 5, P4(5, arrived at 3) has the shortest remaining time and runs during 5-10, then P1 (remaining 7) runs during 10-17 and P3(9) during 17-26. The waiting times are P1 = 10 - 1 = 9, P2 = 0, P3 = 15, P4 = 2, and the average is 26/4 = **6.5 ms**.

   </details>

9. **Fairness of RR:** How does Round-Robin scheduling guarantee fairness? What is the maximum waiting time for a process with n processes and quantum q?

   <details>
   <summary>Answer</summary>

   Every process runs for at most one quantum before being moved to the back of the queue, so no process can monopolize the CPU or starve. The maximum wait before the next run occurs when all other processes each use a full quantum, which is **(n-1) * q**.

   </details>

10. **Choosing the time quantum:** What happens when the time quantum in Round-Robin is very large? Very small? What is the rule of thumb for choosing an appropriate quantum?

    <details>
    <summary>Answer</summary>

    **If it is too large**, RR degenerates into FCFS and responsiveness drops. **If it is too small**, context switch overhead dominates the real work and throughput collapses. The rule of thumb is to set the quantum so that **about 80% of CPU bursts finish within one quantum**, typically 10-100 ms.

    </details>

---
