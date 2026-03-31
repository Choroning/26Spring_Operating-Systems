# W04 Concepts Lecture — Threads and Concurrency (1)

> **Last Updated:** 2026-03-25
>
> Silberschatz, Operating System Concepts Ch 4 (Sections 4.1 – 4.4)

> **Prerequisites**: W02-W03 Process concepts (process, fork, IPC). Basic C programming.
>
> **Learning Objectives**: After reading this note, you should be able to:
> 1. Explain what a thread is and how it differs from a process
> 2. Distinguish concurrency from parallelism
> 3. Compare multithreading models (Many-to-One, One-to-One, Many-to-Many)
> 4. Write basic multithreaded programs using Pthreads
> 5. Apply Amdahl's Law to estimate parallel speedup limits

---

## Table of Contents

- [1. Thread Concepts](#1-thread-concepts)
  - [1.1 What Is a Thread?](#11-what-is-a-thread)
  - [1.2 Single-Threaded vs Multithreaded Processes](#12-single-threaded-vs-multithreaded-processes)
  - [1.3 Thread vs Process Comparison](#13-thread-vs-process-comparison)
  - [1.4 Why Use Threads — Real-World Examples](#14-why-use-threads--real-world-examples)
  - [1.5 Multithreaded Web Server](#15-multithreaded-web-server)
- [2. Benefits of Multithreading](#2-benefits-of-multithreading)
  - [2.1 Four Benefits of Threads](#21-four-benefits-of-threads)
- [3. Multicore Programming](#3-multicore-programming)
  - [3.1 Concurrency vs Parallelism](#31-concurrency-vs-parallelism)
  - [3.2 Single-Core vs Multicore Execution Comparison](#32-single-core-vs-multicore-execution-comparison)
  - [3.3 Five Challenges of Multicore Programming](#33-five-challenges-of-multicore-programming)
  - [3.4 Amdahl's Law](#34-amdahls-law)
  - [3.5 Types of Parallelism](#35-types-of-parallelism)
- [4. User Threads and Kernel Threads](#4-user-threads-and-kernel-threads)
- [5. Multithreading Models](#5-multithreading-models)
  - [5.1 Many-to-One Model](#51-many-to-one-model)
  - [5.2 One-to-One Model](#52-one-to-one-model)
  - [5.3 Many-to-Many Model](#53-many-to-many-model)
  - [5.4 Two-Level Model](#54-two-level-model)
  - [5.5 Multithreading Model Summary](#55-multithreading-model-summary)
- [6. Thread Libraries](#6-thread-libraries)
  - [6.1 Thread Library Overview](#61-thread-library-overview)
  - [6.2 Asynchronous vs Synchronous Threading](#62-asynchronous-vs-synchronous-threading)
  - [6.3 Pthreads Overview](#63-pthreads-overview)
  - [6.4 Pthreads Example — Integer Summation](#64-pthreads-example--integer-summation)
  - [6.5 Pthreads — Creating and Joining Multiple Threads](#65-pthreads--creating-and-joining-multiple-threads)
  - [6.6 pthread_create() Details](#66-pthread_create-details)
  - [6.7 Windows Thread Example](#67-windows-thread-example)
- [7. Java Threads](#7-java-threads)
  - [7.1 Java Thread Overview](#71-java-thread-overview)
  - [7.2 Runnable Interface](#72-runnable-interface)
  - [7.3 Lambda Expressions](#73-lambda-expressions)
  - [7.4 join()](#74-join)
  - [7.5 Comparison with Pthreads/Windows](#75-comparison-with-pthreadswindows)
- [8. Java Executor Framework](#8-java-executor-framework)
  - [8.1 Executor Overview](#81-executor-overview)
  - [8.2 Thread Pool Types](#82-thread-pool-types)
  - [8.3 Benefits of Thread Pools](#83-benefits-of-thread-pools)
  - [8.4 Callable and Future — Returning Results](#84-callable-and-future--returning-results)
  - [8.5 execute() vs submit()](#85-execute-vs-submit)
  - [8.6 JVM and Host OS Relationship](#86-jvm-and-host-os-relationship)
- [9. Lab — Pthreads Multithreaded Programming](#9-lab--pthreads-multithreaded-programming)
  - [9.1 Lab Overview: Splitting Array Summation Across Multiple Threads](#91-lab-overview-splitting-array-summation-across-multiple-threads)
  - [9.2 Thread Function Implementation](#92-thread-function-implementation)
  - [9.3 Main Function](#93-main-function)
  - [9.4 Code Analysis](#94-code-analysis)
  - [9.5 Why the thread_ids Array Is Needed](#95-why-the-thread_ids-array-is-needed)
  - [9.6 Observing Race Conditions](#96-observing-race-conditions)
  - [9.7 Pthreads Integer Summation (Textbook Example Variant)](#97-pthreads-integer-summation-textbook-example-variant)
  - [9.8 Lab Key Takeaways](#98-lab-key-takeaways)
- [Summary](#summary)
- [Appendix](#appendix)

---

<br>

## 1. Thread Concepts

### 1.1 What Is a Thread?

A **Thread** is the **basic unit of CPU utilization** within a process.

> If a process is a factory, then threads are the workers inside it. They share the same building (code, data, files) but each has their own workstation (stack) and tracks their own progress (PC, registers).

What each thread **owns independently**:
- Thread ID
- Program Counter (PC)
- Register Set
- **Stack** (function calls, local variables)

> **Program Counter (PC)**: a register that holds the memory address of the next instruction to be executed.

> **Register Set**: the collection of CPU registers (general-purpose, status, etc.) whose values define a thread's execution state.

What threads within the same process **share**:
- Code section (program code)
- Data section (global variables)
- OS resources (open files, signals, etc.)

> Traditional process = a single thread of control
> Modern process = **multiple threads** of control

> **Key Point:** Threads are sometimes called "lightweight processes." If a process is the **unit of resource ownership** (address space, files, etc.), a thread is the **unit of scheduling** (PC, stack, registers). Multiple execution flows can be created within a single process, saving resources while achieving parallelism.

### 1.2 Single-Threaded vs Multithreaded Processes

![Single-threaded process](../images/figures/fig4_1_single.png) ![Multithreaded process](../images/figures/fig4_1_multi.png)

*Silberschatz, Figure 4.1 — Single-threaded and multithreaded processes*

- Single-threaded: one execution flow, one PC, one stack
- Multithreaded: **multiple execution flows**, each with its own independent PC and stack
- **Code, data, and files are shared across all threads**

> **[Computer Architecture]** Each thread having an independent PC means that each thread can execute different locations in the code simultaneously. During a context switch, the kernel saves/restores the thread's PC, registers, stack pointer, etc.

### 1.3 Thread vs Process Comparison

| Category | Process | Thread |
|----------|---------|--------|
| Creation cost | **High** (memory and resource allocation) | **Low** (only stack and registers needed) |
| Context switching | **Slow** (address space switch) | **Fast** (same address space) |
| Memory sharing | Isolated by default (IPC required) | **Naturally shared** (code, data) |
| Independence | High (one dying has little effect on others) | Low (one dying can affect the entire process) |
| Communication | IPC required (pipes, sockets, etc.) | Direct communication via global variables |

> **IPC (Inter-Process Communication)** was covered in detail in W03. Since threads share the same address space, they can communicate directly through shared memory without IPC.

> Threads are also called **"Lightweight Processes (LWP)."**

> **[Computer Architecture]** Thread context switching is faster than process switching because the address space is the same, so **no TLB flush is needed**. When switching processes, the page table changes, requiring TLB invalidation, which causes frequent TLB misses and degrades performance.
>
> **TLB (Translation Lookaside Buffer)**: a hardware cache that speeds up virtual-to-physical address translation. Thread switches within the same process don't require a TLB flush since they share the same address space.

> **Exam Tip:** "Differences between threads and processes" or "why threads are called lightweight processes" is a frequently tested topic. You should be able to precisely distinguish what is **shared** (code, data, files) and what is **independent** (PC, registers, stack).

### 1.4 Why Use Threads — Real-World Examples

**Web Browser:**
- Thread 1: Receiving data from the network
- Thread 2: Rendering images and text
- Thread 3: Handling user input

**Word Processor:**
- Thread 1: Displaying the document on screen
- Thread 2: Handling keystrokes
- Thread 3: Spell/grammar checking (background)

**Photo App — Thumbnail Generation:**
- A separate thread processes each image in the collection
- Parallel processing reduces total elapsed time

### 1.5 Multithreaded Web Server

![Multithreaded server architecture](../images/figures/fig4_2.png)

*Silberschatz, Figure 4.2 — Multithreaded server architecture*

**Traditional approach**: Create a new **process** per request → time-consuming, resource-wasteful

**Modern approach**: Create a new **thread** per request
- Shares the same address space, so it's **fast and lightweight**
- Advantageous for handling a large number of concurrent connections

> The Linux kernel itself is multithreaded — `kthreadd` (pid=2) is the parent of all kernel threads

> **Note:** Modern high-performance servers have evolved beyond "thread-per-request" to use **event loop** architectures. For example, Nginx uses a few worker threads that handle tens of thousands of concurrent connections via I/O multiplexing with `epoll`/`kqueue`. Node.js also uses a single-threaded event loop. However, "thread-per-request" remains important as a fundamental concept, and Java servlet containers (Tomcat, etc.) use this model.

---

<br>

## 2. Benefits of Multithreading

### 2.1 Four Benefits of Threads

**1. Responsiveness**
- The UI thread can continue responding
- Long-running tasks execute **asynchronously** in a separate thread
- Example: The interface remains responsive during lengthy processing after a button click

**2. Resource Sharing**
- Sharing between processes requires shared memory or message passing
- Threads **share code, data, and files by default**
- Multiple activities can be performed within the same address space

**3. Economy**
- Thread creation is **much cheaper** than process creation
  - Lower memory and resource allocation costs
- Context switching between threads is also **much faster**
  - Same address space, so no TLB flush is needed

**4. Scalability**
- True **parallel execution** is possible on multiprocessor/multicore systems
- A single-threaded process uses only **1 core** regardless of how many cores are available
- A multithreaded process can execute each thread on a different core simultaneously

| Benefit | Key Description | Representative Example |
|---------|----------------|----------------------|
| **Responsiveness** | Program stays responsive during blocking | Web browser UI |
| **Resource Sharing** | Automatic resource sharing within the same address space | Global variable access |
| **Economy** | Lower creation/switching costs than processes | Web server request handling |
| **Scalability** | Parallel execution on multicore | Video encoding, scientific computing |

> When a web server creates threads instead of processes for each request, overhead is significantly reduced.

> **Exam Tip:** The four benefits of threads (Responsiveness, Resource Sharing, Economy, Scalability) are frequently tested in short-answer or essay form. You should be able to explain the **key reason** for each benefit in a single sentence.

---

<br>

## 3. Multicore Programming

### 3.1 Concurrency vs Parallelism

**Concurrency**
- Multiple tasks **making progress** at the same time
- Possible even on a single core — **interleaving** through time-sharing

![Concurrent execution — single core](../images/figures/fig4_3.png)

*Silberschatz, Figure 4.3 — Concurrent execution on a single-core system*

**Parallelism**
- Multiple tasks **actually executing simultaneously**
- Only possible on multicore — each core runs a different thread

![Parallel execution — multicore](../images/figures/fig4_4.png)

*Silberschatz, Figure 4.4 — Parallel execution on a multicore system*

> Concurrency without parallelism is possible, but parallelism without concurrency is not

> Concurrency is one person alternating between cooking and laundry (both in progress, but only one at a time). Parallelism is two people — one cooking, one doing laundry — truly working at the same time.

> **Key Point:** The distinction between concurrency and parallelism must be clearly understood. Concurrency is a **logical concept** that includes tasks alternating execution on a single core. Parallelism is a **physical concept** meaning actual simultaneous execution across multiple cores.

### 3.2 Single-Core vs Multicore Execution Comparison

**Single-Core System (Figure 4.3):**
- 4 threads: T1, T2, T3, T4
- Only one runs at a time → concurrency achieved through **interleaving**
- Not true parallelism

**Multicore System (Figure 4.4):**
- Core 1: Alternates between T1 and T3
- Core 2: Alternates between T2 and T4
- T1 and T2 execute **simultaneously** → **true parallelism**

| Category | Single-Core | Multicore |
|----------|------------|-----------|
| Concurrency | Yes (interleaving) | Yes (parallel) |
| Parallelism | No | **Yes** |
| Performance gain | Only time-sharing | Actual throughput increase |

### 3.3 Five Challenges of Multicore Programming

Challenges that must be overcome to **effectively utilize** multicore systems:

**1. Identifying tasks**
- Must find separable, **independent** tasks
- Ideally, there should be no dependencies between tasks

**2. Balance**
- Must distribute **equal workloads** to each core
- Allocating a separate core to a low-contribution task is inefficient

**3. Data splitting**
- Must **properly partition data** alongside tasks and distribute to each core
- Example: Dividing an array into N equal parts with each core handling one portion

**4. Data dependency**
- If there are data dependencies between tasks, **synchronization** is needed
- If Task B requires the result of Task A, the order must be guaranteed
- Covered in detail in Ch 6

**5. Testing and debugging**
- Parallel execution allows for **various execution paths**
- **Non-deterministic** results can occur
- Hard-to-reproduce bugs (heisenbugs) emerge

> Due to these challenges, many experts argue that the multicore era requires **fundamentally new** software design approaches.

> **Note:** "Heisenbug" is a term derived from Heisenberg's Uncertainty Principle, referring to bugs that disappear when you try to observe (debug) them. A classic example in multithreaded programs is when attaching a debugger changes the timing, preventing the bug from reproducing.

### 3.4 Amdahl's Law

A law that describes the limit of overall performance improvement when only **a portion** of the system is improved.

$$
\text{speedup} \leq \frac{1}{S + \frac{1-S}{N}}
$$

- **S** = serial execution fraction (proportion of the serial portion)
- **N** = number of processing cores

> In this formula, S is the fraction of the program that must execute serially (cannot be parallelized), and N is the number of cores. The denominator represents: serial time (S) + parallel time divided among N cores ((1-S)/N).

Key implications:
- Even as N approaches infinity, the speedup converges to **1/S**
- The serial portion acts as a **bottleneck**

**Numerical examples:**

| S (serial fraction) | N = 2 | N = 4 | N = 8 | N → ∞ |
|:---:|:---:|:---:|:---:|:---:|
| 5% | 1.90x | 3.48x | 5.93x | **20.00x** |
| 10% | 1.82x | 3.08x | 4.71x | **10.00x** |
| 25% | 1.60x | 2.28x | 3.02x | **4.00x** |
| 50% | 1.33x | 1.60x | 1.78x | **2.00x** |

Example: A program that is 75% parallel + 25% serial
- 2 cores → **1.6x** speedup
- 4 cores → **2.28x** speedup
- No matter how many cores are added, the maximum is **4x** (= 1/0.25)

> The larger the serial fraction, the more **limited** the performance improvement from adding cores.

![Amdahl's Law graph](../images/figures/fig4_amdahl.png)

*Silberschatz, Amdahl's Law — Speedup vs. number of cores*

- The smaller S is (higher parallelization ratio), the greater the effect of adding cores
- If S = 0.50, the maximum is 2x regardless of the number of cores

> **Exam Tip:** The Amdahl's Law formula and calculating "speedup on N cores given S" are frequently tested. In particular, remember that **speedup = 1/S when N → ∞**.

> **[Algorithms]** Amdahl's Law is a core concept in parallel algorithm design. No matter how excellent a parallel algorithm is, if there are portions that must execute sequentially (e.g., result merging, initialization), those portions determine the upper bound of speedup.

### 3.5 Types of Parallelism

**Data Parallelism**
- Distributes **the same operation** across **subsets of data**
- Data is partitioned, and each core performs the same task
- Example: Summing an array of size N
  - Core 0: Sum [0] to [N/2-1]
  - Core 1: Sum [N/2] to [N-1]

**Task Parallelism**
- Distributes **different operations (tasks)** to each thread
- Different tasks may operate on the same data
- Example: For an array
  - Core 0: Calculate mean
  - Core 1: Calculate standard deviation

**Data Parallelism vs Task Parallelism Comparison:**

![Data parallelism vs task parallelism](../images/figures/fig4_5.png)

*Silberschatz, Figure 4.5 — Data parallelism vs task parallelism*

| Category | Data Parallelism | Task Parallelism |
|----------|-----------------|------------------|
| Data distribution | **Partitioned** and distributed | Same data possible |
| Operation type | **Same** operation | **Different** operations |
| Scalability | Proportional to data size | Depends on number of tasks |

> In practice, **hybrid** forms are common.

> **Note:** GPU programming (CUDA, OpenCL) is a representative use case of data parallelism. Thousands of cores perform the same operation on different data fragments simultaneously. In contrast, microservice architectures are closer to task parallelism — different services perform different functions.

---

<br>

## 4. User Threads and Kernel Threads

![User threads and kernel threads](../images/figures/fig4_6.png)

*Silberschatz, Figure 4.6 — User threads and kernel threads*

**User Threads:**
- Managed by user-level libraries (without kernel support)
- Examples: POSIX Pthreads, Windows threads, Java threads

> User-level threads are managed entirely by a library in user space — the library maintains its own thread table and performs context switches without kernel involvement. The kernel is unaware of these threads, which means if one user thread makes a blocking system call, all threads in that process may be blocked.

**Kernel Threads:**
- **Directly managed** by the OS kernel
- Supported by virtually all modern OSes: Windows, Linux, macOS

> Key question: **How** should user threads be **mapped** to kernel threads?

> **Key Point:** A common source of confusion with the terms "user threads" and "kernel threads" is that Pthreads and Java threads are ultimately mapped to kernel threads for execution. Here, "user threads" refers to **threads created and managed through user-level APIs**, while "kernel threads" refers to **threads that the kernel's scheduler recognizes and directly schedules**. The mapping model (many-to-one, one-to-one, many-to-many) determines the relationship between the two.

---

<br>

## 5. Multithreading Models

### 5.1 Many-to-One Model

![Many-to-one model](../images/figures/fig4_7.png)

*Silberschatz, Figure 4.7 — Many-to-one model*

Multiple user threads map to **a single** kernel thread.

Advantages:
- Thread management is **efficient** since it's done in user space

Disadvantages:
- If one thread makes a **blocking system call** → **the entire process blocks**
- **True parallelism is impossible** — the kernel can only schedule one thread at a time

Use cases:
- Former Solaris Green Threads
- Early Java (Green Threads)
- **Rarely used** in modern systems

### 5.2 One-to-One Model

![One-to-one model](../images/figures/fig4_8.png)

*Silberschatz, Figure 4.8 — One-to-one model*

Each user thread maps to **one** kernel thread.

Advantages:
- If one thread blocks, **other threads can continue executing**
- **True parallelism** achievable on multiprocessors

Disadvantages:
- Creating a user thread requires **creating a kernel thread** as well → overhead
- Creating too many threads can **degrade system performance**

Use cases:
- Used by **Linux** and **Windows** operating systems
- The most widely used model today

### 5.3 Many-to-Many Model

![Many-to-many model](../images/figures/fig4_9.png)

*Silberschatz, Figure 4.9 — Many-to-many model*

Multiple user threads map to **an equal or smaller number of** kernel threads.

Advantages:
- Developers can create as many user threads as desired
- Kernel threads can execute **in parallel**
- If one thread blocks, the kernel can **schedule another thread**

Disadvantages:
- **Very complex to implement**

> The complexity arises from needing a user-level scheduler that coordinates with the kernel scheduler, plus mechanisms like upcalls for the kernel to notify the user-level scheduler about events.

> Theoretically the most flexible, but in practice the one-to-one model dominates.

### 5.4 Two-Level Model

![Two-level model](../images/figures/fig4_10.png)

*Silberschatz, Figure 4.10 — Two-level model*

**Many-to-many** + allows **one-to-one**

- Most user threads are managed with many-to-many
- Specific **critical threads** can be **directly bound** to kernel threads
  - Example: Threads requiring real-time processing

> In practice, the one-to-one model became standard due to implementation complexity; many-to-many is used internally by some concurrency libraries

### 5.5 Multithreading Model Summary

| Model | Mapping | Advantages | Disadvantages | OS Used |
|-------|---------|------------|---------------|---------|
| Many-to-One | N:1 | Efficient management | Entire process blocks on blocking, no parallelism | Former Solaris |
| **One-to-One** | 1:1 | High concurrency, parallel execution | Thread count limited (overhead) | **Linux, Windows** |
| Many-to-Many | N:M | Flexible, parallel + non-blocking | Complex implementation | Some libraries |
| Two-Level | N:M + 1:1 | Flexible + critical thread binding | Complex implementation | Former Solaris, IRIX |

Key points:
- As processing cores increased, kernel thread count limits became less important
- Most modern OSes adopt the **one-to-one** model

> **Note:** Go's goroutines are an example of a modern many-to-many model. Thousands to millions of goroutines are mapped to a small number of OS threads for execution. Java also introduced Virtual Threads (Project Loom) starting from JDK 21, supporting a similar approach.

---

<br>

## 6. Thread Libraries

### 6.1 Thread Library Overview

A **thread library** provides an **API** for thread creation and management.

Two implementation approaches:

**1. User-level library (user space)**
- All code and data structures reside in user space
- Library function call = **local function call** (not a system call)

**2. Kernel-level library (kernel space)**
- Code and data structures reside in kernel space
- Library function call = **system call**

Three major thread libraries:
1. **POSIX Pthreads** — user-level or kernel-level
2. **Windows Threads** — kernel-level
3. **Java Threads** — runs on the JVM, uses the host OS's library

### 6.2 Asynchronous vs Synchronous Threading

Two main thread creation strategies:

**Asynchronous Threading**
- Once the parent thread creates a child thread, they execute **independently**
- **Little data sharing** between parent and child
- Example: Multithreaded web server — each request handled by an independent thread

**Synchronous Threading**
- The parent thread **waits (joins)** for all child threads to complete
- Child threads **deliver results** to the parent upon completion
- **Significant data sharing** between tasks
- Example: Parallel summation — parent merges partial sums from each thread

> All examples in this chapter follow the **synchronous threading** pattern.

### 6.3 Pthreads Overview

**POSIX Pthreads** = IEEE 1003.1c standard

> This is a **specification**, not an **implementation**
> OS designers are free to choose their implementation approach

Key API functions:

| Function | Description |
|----------|-------------|
| `pthread_create()` | Create a new thread |
| `pthread_join()` | Wait for thread termination |
| `pthread_exit()` | Terminate the current thread |
| `pthread_attr_init()` | Initialize thread attributes |

Supported environments: UNIX, Linux, macOS
- Windows does not natively support it (third-party implementations exist)

### 6.4 Pthreads Example — Integer Summation (Figure 4.11)

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sum;  /* Global variable: shared data between threads */
void *runner(void *param);  /* Function to be executed by the thread */

int main(int argc, char *argv[])
{
    pthread_t tid;           /* Thread identifier */
    pthread_attr_t attr;     /* Thread attributes */

    pthread_attr_init(&attr);           /* Initialize default attributes */
    pthread_create(&tid, &attr,         /* Create thread */
                   runner, argv[1]);
    pthread_join(tid, NULL);            /* Wait for thread completion */

    printf("sum = %d\n", sum);
}

void *runner(void *param)
{
    int i, upper = atoi(param);
    sum = 0;
    for (i = 1; i <= upper; i++)
        sum += i;
    pthread_exit(0);  /* Terminate thread */
}
```

**Execution flow** (input = 5):

1. `main()` calls `pthread_attr_init(&attr)` — set default attributes
2. `pthread_create(&tid, &attr, runner, argv[1])`
   - Creates a new thread, starts execution at `runner("5")`
3. `pthread_join(tid, NULL)` — parent **waits** for child to complete
4. Child thread (`runner`):
   - `upper = 5`, sum = 1+2+3+4+5 = **15**
   - Terminates with `pthread_exit(0)`
5. Parent: `printf("sum = %d\n", sum)` → prints **"sum = 15"**

**Key points:**
- `sum` is a **global variable** → shared by all threads
- Without `pthread_join()`, the parent might terminate first
- `runner()` takes a `void *` parameter and returns `void *`

### 6.5 Pthreads — Creating and Joining Multiple Threads

**Multiple thread join pattern (Figure 4.12):**

```c
#define NUM_THREADS 10

pthread_t workers[NUM_THREADS];

/* Create threads */
for (int i = 0; i < NUM_THREADS; i++)
    pthread_create(&workers[i], NULL, task_func, &args[i]);

/* Wait for all threads to complete */
for (int i = 0; i < NUM_THREADS; i++)
    pthread_join(workers[i], NULL);
```

- The creation loop and join loop are **separated**
- All threads execute **in parallel**, then are joined sequentially
- A commonly used pattern on multicore systems

### 6.6 pthread_create() Details

```c
int pthread_create(
    pthread_t *thread,             /* Variable to store thread ID */
    const pthread_attr_t *attr,    /* Thread attributes (NULL = defaults) */
    void *(*start_routine)(void*), /* Function for the thread to execute */
    void *arg                      /* Argument to pass to the function */
);
```

**Parameter descriptions:**

| Parameter | Role |
|-----------|------|
| `thread` | Stores the ID of the newly created thread |
| `attr` | Stack size, scheduling info, etc. (defaults if NULL) |
| `start_routine` | Function pointer of the form `void *func(void *param)` |
| `arg` | Argument to be passed to start_routine (void * type) |

> Reading the declaration `void *(*start_routine)(void*)`: `start_routine` is a pointer to a function that takes a `void *` parameter and returns `void *`. The `void *` type acts as a generic pointer — it can point to any data type.

Return value: 0 on success, error number on failure

### 6.7 Windows Thread Example (Figure 4.13)

```c
#include <windows.h>
#include <stdio.h>

DWORD Sum;  /* Shared data (unsigned 32-bit integer) */

DWORD WINAPI Summation(LPVOID Param)
{
    DWORD Upper = *(DWORD*)Param;
    for (DWORD i = 1; i <= Upper; i++)
        Sum += i;
    return 0;
}

int main(int argc, char *argv[])
{
    DWORD ThreadId;
    HANDLE ThreadHandle;
    int Param = atoi(argv[1]);

    ThreadHandle = CreateThread(
        NULL,        /* Default security attributes */
        0,           /* Default stack size */
        Summation,   /* Thread function */
        &Param,      /* Parameter to pass to thread function */
        0,           /* Default creation flags */
        &ThreadId);  /* Returns thread identifier */

    WaitForSingleObject(ThreadHandle, INFINITE);
    CloseHandle(ThreadHandle);
    printf("sum = %d\n", Sum);
}
```

**Comparison with Pthreads:**

| Category | Pthreads | Windows Threads |
|----------|---------|-----------------|
| Header | `<pthread.h>` | `<windows.h>` |
| Thread creation | `pthread_create()` | `CreateThread()` |
| Wait for termination | `pthread_join()` | `WaitForSingleObject()` |
| Wait for multiple | for loop + `pthread_join()` | `WaitForMultipleObjects()` |
| Thread termination | `pthread_exit()` | `return 0;` or `ExitThread()` |
| Handle cleanup | Automatic | `CloseHandle()` required |
| Shared data | Global variables | Global variables (DWORD, etc.) |

**WaitForMultipleObjects() example:**
```c
/* Wait until all N thread handles in the array have completed */
WaitForMultipleObjects(N, THandles, TRUE, INFINITE);
```

---

<br>

## 7. Java Threads

> **Note:** Java threads are presented here for comparison with Pthreads. In this course's labs, we primarily use Pthreads. Focus on understanding the concepts; the Java syntax is secondary.

### 7.1 Java Thread Overview

In Java, threads are the **fundamental model of program execution**.
- Every Java program runs in at least one thread (the main thread)
- Runs on the **JVM** → uses the host OS's thread library
  - Windows → Windows API
  - Linux/macOS → Pthreads API

Two ways to create threads:

1. **Extend the Thread class** and override `run()`
2. **Implement the Runnable interface** (more common)

> The Runnable approach is recommended — because Java does not support multiple inheritance

### 7.2 Runnable Interface

**Method 1: Implement the Runnable interface**

```java
class Task implements Runnable {
    public void run() {
        System.out.println("I am a thread.");
    }
}

// Create and start a thread
Thread worker = new Thread(new Task());
worker.start();  // Executes run() in a new thread
```

What the `start()` method does:
1. **Allocates memory and initializes** a new thread in the JVM
2. Calls the `run()` method to make the thread **runnable**

> Note: If you call `run()` directly, it executes in the **same thread**!
> You must call `start()` to create a new thread.

### 7.3 Lambda Expressions

**Method 2: Lambda expressions (Java 8+)**

Runnable is a **functional interface** with only one abstract method → lambda can be used

```java
// Concisely expressed with lambda
Runnable task = () -> {
    System.out.println("I am a thread.");
};

Thread worker = new Thread(task);
worker.start();
```

Even more concise:

```java
new Thread(() -> {
    System.out.println("I am a thread.");
}).start();
```

> Using lambda expressions allows defining thread tasks inline without a separate class definition, making **code more concise**.

### 7.4 join()

To wait for a child thread to complete, the parent uses `join()`:

```java
Thread worker = new Thread(new Task());
worker.start();

try {
    worker.join();  // Wait until worker finishes
} catch (InterruptedException ie) {
    // Handle interrupt during wait
}

System.out.println("Worker finished!");
```

**Multiple thread join pattern:**

```java
Thread[] workers = new Thread[10];
for (int i = 0; i < 10; i++) {
    workers[i] = new Thread(new Task());
    workers[i].start();
}
for (int i = 0; i < 10; i++) {
    try { workers[i].join(); }
    catch (InterruptedException ie) { }
}
```

### 7.5 Comparison with Pthreads/Windows

| Category | Pthreads (C) | Windows (C) | Java |
|----------|-------------|-------------|------|
| Creation | `pthread_create()` | `CreateThread()` | `new Thread().start()` |
| Waiting | `pthread_join()` | `WaitForSingleObject()` | `thread.join()` |
| Termination | `pthread_exit()` | `return` / `ExitThread()` | `run()` returns |
| Shared data | Global variables | Global variables | **Object fields** (no global variables) |
| Returning results | Difficult (`void *`) | Difficult | **Callable/Future** |

Java characteristics:
- **No concept of global data**
- Data sharing between threads is **explicitly set up through objects**
- The JVM internally uses the host OS's thread library

---

<br>

## 8. Java Executor Framework

### 8.1 Executor Overview

Available since Java 5 in the `java.util.concurrent` package.

**Executor interface:**
```java
public interface Executor {
    void execute(Runnable command);
}
```

Difference from the traditional approach:
```java
// Traditional: Create thread directly
Thread t = new Thread(new Task());
t.start();

// Executor approach: Separate thread creation from execution
Executor service = new SomeExecutor();
service.execute(new Task());
```

> **Producer-consumer model**: Tasks (Runnables) are produced, and threads consume them for execution

> **[Programming Languages]** The Executor pattern is an example of the **Strategy Pattern**. By separating the task (what to do) from the execution strategy (how to execute), the same task can be run in various ways: single thread, thread pool, scheduling, etc.

### 8.2 Thread Pool Types

Factory methods of the `Executors` class:

| Method | Description |
|--------|-------------|
| `newSingleThreadExecutor()` | Thread pool of size 1 — guarantees sequential execution |
| `newFixedThreadPool(int n)` | Maintains a fixed pool of n threads |
| `newCachedThreadPool()` | Creates threads as needed, **reuses** idle threads |

```java
// Fixed thread pool example
ExecutorService pool = Executors.newFixedThreadPool(4);

for (int i = 0; i < 100; i++) {
    pool.execute(new Task());  // 100 tasks, processed by 4 threads
}

pool.shutdown();  // Shut down pool after all tasks complete
```

> Using thread pools reduces thread creation/destruction costs and **limits** the number of concurrent threads.

### 8.3 Benefits of Thread Pools

**1. Fast response through reusing existing threads**
- Saves the cost of creating threads each time
- Already existing threads perform tasks immediately

**2. Limiting the number of threads**
- Prevents excessive system resource usage
- Fixed pool: Only up to n threads execute concurrently

**3. Separation of task and execution mechanism**
- Separates task definition (what to do) from execution strategy (how to execute)
- Various strategies possible: delayed execution, periodic execution, etc.

> Pool size should be configured considering **CPU count, memory, and expected concurrent requests**

### 8.4 Callable and Future — Returning Results

**Problem**: Runnable's `run()` has no return value (`void`)

**Solution**: `Callable<V>` interface + `Future<V>`

```java
import java.util.concurrent.*;

class Summation implements Callable<Integer> {
    private int upper;

    public Summation(int upper) {
        this.upper = upper;
    }

    public Integer call() {
        int sum = 0;
        for (int i = 1; i <= upper; i++)
            sum += i;
        return new Integer(sum);
    }
}
```

- `Callable`'s `call()` method can **return a result**
- The returned result is received through a `Future` object

**Full example (Figure 4.14):**

```java
import java.util.concurrent.*;

class Summation implements Callable<Integer> {
    private int upper;
    public Summation(int upper) { this.upper = upper; }

    public Integer call() {
        int sum = 0;
        for (int i = 1; i <= upper; i++)
            sum += i;
        return new Integer(sum);
    }
}

public class Driver {
    public static void main(String[] args) {
        int upper = Integer.parseInt(args[0]);
        ExecutorService pool =
            Executors.newSingleThreadExecutor();
        Future<Integer> result =
            pool.submit(new Summation(upper));

        try {
            System.out.println("sum = " + result.get());
        } catch (InterruptedException |
                 ExecutionException ie) { }
    }
}
```

- `submit()` → Submits a task and returns a **Future**
- `result.get()` → **Blocks** until the result is ready

### 8.5 execute() vs submit()

| Method | Parameter | Return Value | When to Use |
|--------|-----------|-------------|-------------|
| `execute(Runnable)` | Runnable | None (`void`) | Tasks that don't need results |
| `submit(Callable)` | Callable | `Future<V>` | Tasks that **need results** |
| `submit(Runnable)` | Runnable | `Future<?>` | Only checking completion status |

**Thread pool example (Figure 4.15):**

```java
import java.util.concurrent.*;

public class ThreadPoolExample {
    public static void main(String[] args) {
        int numTasks = Integer.parseInt(args[0].trim());
        ExecutorService pool = Executors.newCachedThreadPool();

        for (int i = 0; i < numTasks; i++)
            pool.execute(new Task());

        pool.shutdown();  // Reject new tasks, shut down after existing tasks complete
    }
}
```

### 8.6 JVM and Host OS Relationship

```text
 ┌──────────────────────────────┐
 │      Java Application        │
 │   (Thread, Executor, ...)    │
 ├──────────────────────────────┤
 │             JVM              │
 │     Java Thread API impl.    │
 ├──────────────────────────────┤
 │           Host OS            │
 │    Windows → Windows API     │
 │    Linux/macOS → Pthreads    │
 └──────────────────────────────┘
```

- The JVM specification does not prescribe **how** Java threads are **mapped** to the OS
- It depends on the implementation:
  - **Windows**: One-to-one model → each Java thread maps to a kernel thread
  - **Linux/macOS**: Maps using the Pthreads API
- As a result, Java threads are also managed by the **OS scheduler**

---

<br>

## 9. Lab — Pthreads Multithreaded Programming

### 9.1 Lab Overview: Splitting Array Summation Across Multiple Threads

**Goal**: Split the summation of a 1000-element array across 4 threads

**Applying data parallelism:**
- Thread 0: Sum [0] to [249]
- Thread 1: Sum [250] to [499]
- Thread 2: Sum [500] to [749]
- Thread 3: Sum [750] to [999]

```text
 Array: [  0 ~ 249  |  250 ~ 499  |  500 ~ 749  |  750 ~ 999  ]
             ↓             ↓             ↓              ↓
          Thread 0     Thread 1       Thread 2       Thread 3
          partial[0]   partial[1]     partial[2]     partial[3]
              ↘            ↓              ↓            ↙
                               Total Sum
```

### 9.2 Thread Function Implementation

```c
#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 4
#define ARRAY_SIZE  1000

int array[ARRAY_SIZE];
int partial_sum[NUM_THREADS];

void *sum_array(void *arg)
{
    int id    = *(int *)arg;
    int chunk = ARRAY_SIZE / NUM_THREADS;
    int start = id * chunk;
    int end   = start + chunk;

    partial_sum[id] = 0;
    for (int i = start; i < end; i++)
        partial_sum[id] += array[i];

    printf("Thread %d: partial_sum = %d\n",
           id, partial_sum[id]);
    pthread_exit(NULL);
}
```

- Each thread handles a **different segment** of the array based on `id`
- Results are stored in `partial_sum[id]` (**separate index** for each thread)

### 9.3 Main Function

```c
int main()
{
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    /* Initialize array: array[i] = i + 1 */
    for (int i = 0; i < ARRAY_SIZE; i++)
        array[i] = i + 1;

    /* Create threads */
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL,
                       sum_array, &thread_ids[i]);
    }

    /* Wait for all threads and accumulate results */
    int total = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total += partial_sum[i];
    }

    printf("Total sum = %d\n", total);
    /* Expected value: 1+2+...+1000 = 500500 */
    return 0;
}
```

### 9.4 Code Analysis

**Execution flow:**
1. `array[i] = i + 1` → [1, 2, 3, ..., 1000]
2. 4 threads created, each handling 250 elements
3. Thread 0: 1+2+...+250 = 31375
4. Thread 1: 251+252+...+500 = 93875
5. Thread 2: 501+502+...+750 = 156375
6. Thread 3: 751+752+...+1000 = 218875
7. `pthread_join()` waits for all threads to complete
8. Total = 31375 + 93875 + 156375 + 218875 = **500500**

**Compiling and running:**
```text
gcc -pthread lab_sum.c -o lab_sum
./lab_sum
```

> The `-pthread` flag is required (links the Pthreads library)

### 9.5 Why the thread_ids Array Is Needed

**Incorrect code** (common mistake):
```c
for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL,
                   sum_array, &i);  /* Dangerous! */
}
```

**Problem:**
- All threads share **the same address of variable `i`**
- By the time a thread runs, `i`'s value may have already changed
- Example: All 4 threads might read `id = 4`

**Correct code:**
```c
int thread_ids[NUM_THREADS];
for (int i = 0; i < NUM_THREADS; i++) {
    thread_ids[i] = i;  /* Separate variable for each thread */
    pthread_create(&threads[i], NULL,
                   sum_array, &thread_ids[i]);
}
```

> This is a classic example of a **data dependency** problem.

> **Exam Tip:** "Why can't you pass `&i` directly?" is a frequently tested Pthreads question. The key is that `pthread_create()` creates threads asynchronously, so by the time the thread actually reads `*(int*)arg`, the value of `i` may have already changed.

### 9.6 Observing Race Conditions

> Imagine two people checking the same bank account balance simultaneously and each withdrawing 10,000 won. Both see 50,000 won, both withdraw, but only one withdrawal is recorded — the bank loses money.

What if all threads share a **single global variable** instead of `partial_sum[id]`?

```c
int global_sum = 0;  /* Shared by all threads */

void *sum_array_bad(void *arg) {
    int id    = *(int *)arg;
    int start = id * (ARRAY_SIZE / NUM_THREADS);
    int end   = start + (ARRAY_SIZE / NUM_THREADS);

    for (int i = start; i < end; i++)
        global_sum += array[i];  /* Race condition! */

    pthread_exit(NULL);
}
```

A **race condition** occurs:
- `global_sum += array[i]` is a 3-step read-modify-write operation
- If multiple threads execute simultaneously, **values can be lost**
- Running multiple times can produce **different results**

> Synchronization is covered in detail in Ch 6 and Ch 7

> **Key Point:** The root cause of the race condition is that `global_sum += array[i]` is **not an atomic operation**. This single line of C code actually executes in three steps: (1) LOAD `global_sum` into a register, (2) ADD `array[i]`, (3) STORE the result back to `global_sum`. If two threads read the same value simultaneously, one thread's update gets overwritten.

### 9.7 Pthreads Integer Summation (Textbook Example Variant)

Sums integers received as a command-line argument:

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sum = 0;

void *runner(void *param)
{
    int upper = atoi(param);
    for (int i = 1; i <= upper; i++)
        sum += i;
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <integer>\n", argv[0]);
        return 1;
    }
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, runner, argv[1]);
    pthread_join(tid, NULL);
    printf("sum = %d\n", sum);
    return 0;
}
```

Execution: `./a.out 10` → "sum = 55"

### 9.8 Lab Key Takeaways

**Basic Pthreads pattern:**
1. `pthread_attr_init()` — Initialize attributes (defaults are sufficient)
2. `pthread_create()` — Create thread (function pointer + argument)
3. Perform work in the thread function (share results via global variables)
4. `pthread_join()` — Wait for termination
5. Collect and print results

**Precautions:**
- Use **separate variables** when passing arguments to thread functions (to prevent race conditions)
- **Synchronization** is needed when accessing shared variables (avoided in this lab by using separate indices)
- The `-pthread` flag is required when compiling

---

<br>

## Summary

| Concept | Key Summary |
|:--------|:-----------|
| Thread | Unit of execution within a process; shares code/data/files; PC/registers/stack are independent |
| Benefits of Threads | Responsiveness, resource sharing, economy, scalability |
| Multicore Programming | Concurrency vs parallelism; 5 challenges (identifying tasks, balance, data splitting, data dependency, testing) |
| Amdahl's Law | Serial fraction is the bottleneck — speedup ≤ 1/S; even with N → ∞, cannot exceed 1/S |
| Data Parallelism vs Task Parallelism | Partition data for the same operation vs distribute different tasks |
| Multithreading Models | Many-to-one (rarely used), **one-to-one** (Linux, Windows), many-to-many (complex) |
| Pthreads | `pthread_create()`, `pthread_join()`, `pthread_exit()`; POSIX standard |
| Windows Threads | `CreateThread()`, `WaitForSingleObject()`, `CloseHandle()` |
| Java Threads | `Runnable`, `Thread.start()`, `Thread.join()`, lambda |
| Java Executor Framework | Separates task from execution with thread pools; returns results via `Callable`/`Future` |
| Race Condition | Occurs with non-atomic read-modify-write on shared variables; synchronization needed (Ch 6) |
| Textbook Scope | Silberschatz Ch 4, Sections 4.1–4.4 |

---

<br>

## Appendix

- **Next Week — Week 5: Threads and Concurrency 2 (Ch 4 second half)**
  - Implicit Threading
    - Thread Pools, Fork-Join, OpenMP, Grand Central Dispatch (GCD)
  - Threading Issues
    - fork() / exec() in multithreaded programs
    - Signal handling
    - Thread cancellation
    - Thread-Local Storage (TLS)
  - OS-specific thread implementations (Linux, Windows)
  - Textbook: Ch 4 (Sections 4.5 – 4.8)

---

<br>

## Self-Check Questions

1. What do threads in the same process share? What does each thread own independently?
2. Why doesn't a thread switch within the same process require a TLB flush?
3. If a program has 20% serial code, what is the maximum speedup with 8 cores according to Amdahl's Law? What about with infinite cores?
4. Explain the difference between concurrency and parallelism with an example.
5. Why is the one-to-one multithreading model the most widely used in modern operating systems, despite its overhead of creating a kernel thread for each user thread?

---
