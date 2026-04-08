# Week 5 Concepts Lecture — Threads and Concurrency (2)

> **Last Updated:** 2026-04-01
>
> Silberschatz, Operating System Concepts Ch 4 (Sections 4.5 – 4.7)

> **Prerequisites**: Week 4 Thread concepts (thread, Pthreads, Java threads, multithreading models). Understanding of thread pools and basic Java concurrency.
>
> **Learning Objectives**:
> 1. Explain what implicit threading is and why it is needed
> 2. Compare five implicit threading techniques (Thread Pool, Fork-Join, OpenMP, GCD, Intel TBB)
> 3. Analyze threading issues: fork()/exec() semantics, signal handling, thread cancellation, TLS, scheduler activations
> 4. Describe how Windows and Linux implement threads at the OS level
> 5. Write parallel programs using OpenMP directives

---

## Table of Contents

- [1. Implicit Threading](#1-implicit-threading)
  - [1.1 Implicit Threading Overview](#11-implicit-threading-overview)
  - [1.2 Task vs Thread](#12-task-vs-thread)
- [2. Thread Pools](#2-thread-pools)
  - [2.1 Thread Pool Concept](#21-thread-pool-concept)
  - [2.2 Thread Pool Benefits](#22-thread-pool-benefits)
  - [2.3 Thread Pool — Windows API](#23-thread-pool--windows-api)
  - [2.4 Thread Pool — Java ExecutorService](#24-thread-pool--java-executorservice)
  - [2.5 Java ExecutorService — Details](#25-java-executorservice--details)
- [3. Fork-Join Parallelism](#3-fork-join-parallelism)
  - [3.1 Fork-Join Concept](#31-fork-join-concept)
  - [3.2 Fork-Join and Divide-and-Conquer](#32-fork-join-and-divide-and-conquer)
  - [3.3 Fork-Join in Java — ForkJoinPool](#33-fork-join-in-java--forkjoinpool)
  - [3.4 Fork-Join in Java — SumTask Implementation](#34-fork-join-in-java--sumtask-implementation)
  - [3.5 Work Stealing](#35-work-stealing)
- [4. OpenMP](#4-openmp)
  - [4.1 OpenMP Overview](#41-openmp-overview)
  - [4.2 parallel for](#42-parallel-for)
  - [4.3 Additional Features](#43-additional-features)
  - [4.4 reduction Example](#44-reduction-example)
- [5. Grand Central Dispatch (GCD)](#5-grand-central-dispatch-gcd)
  - [5.1 GCD Overview](#51-gcd-overview)
  - [5.2 Serial vs Concurrent Queue](#52-serial-vs-concurrent-queue)
  - [5.3 Blocks and Closures](#53-blocks-and-closures)
  - [5.4 GCD Internal Architecture](#54-gcd-internal-architecture)
- [6. Intel Thread Building Blocks (TBB)](#6-intel-thread-building-blocks-tbb)
  - [6.1 Intel TBB Overview](#61-intel-tbb-overview)
  - [6.2 parallel_for Example](#62-parallel_for-example)
  - [6.3 parallel_reduce Example](#63-parallel_reduce-example)
- [7. Implicit Threading — Comparison of 5 Techniques](#7-implicit-threading--comparison-of-5-techniques)
- [8. Threading Issues](#8-threading-issues)
  - [8.1 Threading Issues Overview](#81-threading-issues-overview)
  - [8.2 fork() and exec() Semantics](#82-fork-and-exec-semantics)
  - [8.3 Signal Handling — Concepts and Patterns](#83-signal-handling--concepts-and-patterns)
  - [8.4 Signal Handling — Multithreaded Environment](#84-signal-handling--multithreaded-environment)
  - [8.5 Thread Cancellation — Concept](#85-thread-cancellation--concept)
  - [8.6 Asynchronous vs Deferred Cancellation](#86-asynchronous-vs-deferred-cancellation)
  - [8.7 Thread Cancellation — Pthreads API](#87-thread-cancellation--pthreads-api)
  - [8.8 Cancellation Points and Cleanup Handlers](#88-cancellation-points-and-cleanup-handlers)
  - [8.9 Thread Cancellation — Java](#89-thread-cancellation--java)
  - [8.10 Thread-Local Storage (TLS)](#810-thread-local-storage-tls)
  - [8.11 TLS — Implementation by Language](#811-tls--implementation-by-language)
  - [8.12 Scheduler Activations — LWP and Upcalls](#812-scheduler-activations--lwp-and-upcalls)
  - [8.13 Scheduler Activations — Upcall Mechanism](#813-scheduler-activations--upcall-mechanism)
- [9. Operating-System Examples](#9-operating-system-examples)
  - [9.1 Windows Threads — Overview](#91-windows-threads--overview)
  - [9.2 Windows Threads — Data Structures](#92-windows-threads--data-structures)
  - [9.3 Linux Threads — Task Concept](#93-linux-threads--task-concept)
  - [9.4 Linux — clone() System Call](#94-linux--clone-system-call)
  - [9.5 Linux — fork() vs clone() Comparison](#95-linux--fork-vs-clone-comparison)
- [10. Lab — OpenMP Parallel Programming](#10-lab--openmp-parallel-programming)
  - [10.1 Lab: OpenMP Matrix Multiplication Parallelization](#101-lab-openmp-matrix-multiplication-parallelization)
  - [10.2 Lab: Performance Comparison by Thread Count](#102-lab-performance-comparison-by-thread-count)
  - [10.3 Lab: OpenMP reduction Lab](#103-lab-openmp-reduction-lab)
- [Summary](#summary)
- [Appendix](#appendix)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. Implicit Threading

### 1.1 Implicit Threading Overview

**Implicit Threading** is a strategy that delegates thread creation and management to the **compiler and runtime library**.

> If explicit threading (Week 4) is like a chef personally managing every worker in the kitchen — assigning tasks, watching timing, handling conflicts — then implicit threading is like hiring a restaurant manager who handles all staffing and scheduling while the chef only decides what dishes to make.

**Why is it needed?**
- Applications with hundreds to thousands of threads have emerged
- Guaranteeing **correctness** of multithreaded programming is extremely difficult
- Direct thread management by programmers leads to exploding complexity

**Core idea**: Developers only **identify parallelizable tasks** → the library handles thread mapping

**Five main techniques**:
1. Thread Pools
2. Fork-Join Parallelism
3. OpenMP
4. Grand Central Dispatch (GCD)
5. Intel Thread Building Blocks (TBB)

> **Key Point:** The shift from explicit to implicit threading reflects a broader trend in systems programming: **abstracting away low-level details** so developers can focus on *what* to parallelize rather than *how* to manage threads. This is similar to how garbage-collected languages freed developers from manual memory management.

### 1.2 Task vs Thread

**Task-based approach**:
- Tasks are typically written as **functions**
- The runtime library maps tasks to separate threads
- Generally utilizes the **Many-to-Many model**
- Developers do not need to worry about thread count, creation timing, or scheduling

```text
┌──────────────────────────────────────┐
│          Application Code            │
│    (identify parallel tasks)         │
├──────────────────────────────────────┤
│     Compiler / Runtime Library       │
│  (create threads, assign tasks,      │
│   manage scheduling)                 │
├──────────────────────────────────────┤
│         Operating System             │
│    (kernel thread management)        │
└──────────────────────────────────────┘
```

> This layered architecture means that the application programmer operates at the highest level of abstraction. The compiler/runtime layer acts as a "smart translator" — it knows how many cores are available, the current system load, and makes decisions about thread creation and scheduling that would be impractical for a programmer to manage manually in every application.

---

<br>

## 2. Thread Pools

### 2.1 Thread Pool Concept

**Problem scenario** (web server example):
- What if a new thread is created for every request?
  - Thread creation takes time → destroyed after completion (wasteful)
  - **No limit** on the number of concurrent threads → risk of system resource exhaustion

**Thread Pool solution**:
- **Pre-create a fixed number of threads** at startup and keep them waiting in the pool
- When a request arrives, **wake an idle thread** from the pool to handle the task
- If no idle threads are available → task **waits in a queue**
- After task completion, the thread is **returned** to the pool for reuse

```text
Request ──→ [ Task Queue ] ──→ ┌─ Thread 1 (busy) ─┐
Request ──→ [ Task Queue ] ──→ │  Thread 2 (idle)   │ ──→ Execute
Request ──→ [ Task Queue ] ──→ │  Thread 3 (busy)   │
                               │  Thread 4 (idle)   │
                               └─ Thread Pool ──────┘
```

> Think of a thread pool like a taxi stand at an airport. Instead of manufacturing a new car for every passenger and scrapping it after one ride (creating/destroying threads), a fleet of taxis waits at the stand. When a passenger arrives, an available taxi takes them; when done, it returns to the stand. If all taxis are busy, passengers wait in a queue.

### 2.2 Thread Pool Benefits

**Three textbook benefits** (Silberschatz, p.178):

1. **Speed**: Reusing existing threads → eliminates wait time for new thread creation
2. **Resource bound**: **Bounds** the number of concurrent threads → prevents system overload
3. **Separation of execution and creation**: Separates task execution from creation mechanism
   - Enables various strategies such as delayed execution, periodic execution, etc.

**Determining pool size**:
- Set **heuristically** based on CPU count, physical memory, expected concurrent requests
- Can be dynamically adjusted (e.g., GCD automatically adjusts based on system load)

> **Exam Tip:** The three benefits of thread pools (Speed, Resource bound, Separation of execution and creation) map closely to the four benefits of threads from Week 4 (Responsiveness, Resource Sharing, Economy, Scalability). Thread pools amplify the Economy benefit further by eliminating repeated creation/destruction overhead.

### 2.3 Thread Pool — Windows API

Windows provides a thread pool API, used similarly to `Thread_Create()`:

```c
// Define a function to be executed as a separate thread
DWORD WINAPI PoolFunction(PVOID Param) {
    /* this function runs as a separate thread. */
}

// Submit work to the thread pool
QueueUserWorkItem(&PoolFunction, NULL, 0);
```

**Three parameters of `QueueUserWorkItem()`**:
1. `LPTHREAD_START_ROUTINE Function` — function pointer to execute as a separate thread
2. `PVOID Param` — parameter to pass to the function
3. `ULONG Flags` — flags specifying how the thread pool creates/manages threads

Additional utilities: **periodic execution**, **callback on asynchronous I/O completion**, etc.

### 2.4 Thread Pool — Java ExecutorService

Three thread pool architectures provided by the `java.util.concurrent` package:

| Factory Method | Description |
|---------------|-------------|
| `newSingleThreadExecutor()` | Creates a pool of size 1 |
| `newFixedThreadPool(int size)` | Creates a fixed-size pool |
| `newCachedThreadPool()` | Unbounded pool, reuses idle threads |

```java
import java.util.concurrent.*;

public class ThreadPoolExample {
    public static void main(String[] args) {
        int numTasks = Integer.parseInt(args[0].trim());

        /* Create the thread pool */
        ExecutorService pool = Executors.newCachedThreadPool();

        /* Run each task using a thread in the pool */
        for (int i = 0; i < numTasks; i++)
            pool.execute(new Task());

        /* Shut down the pool once all threads have completed */
        pool.shutdown();
    }
}
```

*Figure 4.15 — Creating a thread pool in Java (textbook p.180)*

> This example was introduced in Week 4's Java Executor Framework section. Here the key difference is **perspective**: in Week 4 we learned the Executor API as a thread library; now we see thread pools as an **implicit threading technique** where the library manages threads for us.

### 2.5 Java ExecutorService — Details

**ExecutorService interface structure**:

```text
      Executor (interface)
          │
          ▼
    ExecutorService (interface)
          │
    ┌─────┴─────┐
    │            │
execute()    submit()    shutdown()
(Runnable)   (Callable)  (shutdown request)
```

**execute() vs submit()**:
- `execute(Runnable)` — executes a task with no return value
- `submit(Callable<T>)` — returns a `Future<T>`, result can be retrieved later

**When shutdown() is called**:
- Rejects additional task submissions
- Waits for all currently running tasks to complete before terminating

**Note — Android AIDL**: Provides a thread pool to remote services, handling multiple concurrent requests with individual threads from the pool

> **[Mobile Development]** In Android, AIDL (Android Interface Definition Language) enables inter-process communication. When a client calls a remote service method, the AIDL framework dispatches the call to a thread from the service's thread pool — the developer doesn't need to create threads manually. This is another example of implicit threading at the framework level.

---

<br>

## 3. Fork-Join Parallelism

### 3.1 Fork-Join Concept

**Fork-Join model**:
- The main (parent) thread **forks** (creates) one or more child tasks
- Child tasks execute in parallel
- Once all children complete, they **join** (merge) and results are combined

**From the implicit threading perspective**:
- Instead of creating threads directly, **parallel tasks are specified**
- The library determines the number of threads and assigns tasks to them
- A variant of a synchronous thread pool

*Silberschatz, Figure 4.16 — Fork-join parallelism*

> The fork-join model is like a team leader delegating subtasks. The leader (parent thread) "forks" work to team members, then waits at a meeting point for everyone to "join" back with their results. The fork-join library is the project manager who decides how many people to hire and how to assign tasks efficiently.

### 3.2 Fork-Join and Divide-and-Conquer

Fork-Join is particularly well-suited for **recursive divide-and-conquer** algorithms:

```text
Task(problem):
    if problem is small enough:
        solve the problem directly
    else:
        subtask1 = fork(new Task(subset of problem))
        subtask2 = fork(new Task(subset of problem))
        result1 = join(subtask1)
        result2 = join(subtask2)
        return combined results
```

*Silberschatz, Figure 4.17 — Fork-join in Java*

> **[Algorithms]** This is the same pattern as merge sort and quicksort. The key insight is that divide-and-conquer naturally maps to fork-join: each recursive call becomes a forked task, and the merge step is the join. The difference is that fork-join **actually runs subtasks in parallel** across threads, whereas a regular recursive call executes sequentially on the stack.

### 3.3 Fork-Join in Java — ForkJoinPool

Fork-Join framework introduced in Java 1.7:
- **ForkJoinPool**: worker thread pool
- **ForkJoinTask\<V\>**: abstract base class

```java
ForkJoinPool pool = new ForkJoinPool();
// array contains the integers to be summed
int[] array = new int[SIZE];
SumTask task = new SumTask(0, SIZE - 1, array);
int sum = pool.invoke(task);
```

**Class hierarchy** (Figure 4.19):

```text
      ForkJoinTask<V> (abstract)
              │
      ┌───────┴────────┐
      │                 │
RecursiveTask<V>   RecursiveAction
  (returns V)      (returns void)
```

*Silberschatz, Figure 4.19 — UML class diagram for Fork-Join*

> `RecursiveTask<V>` is used when you need to return a computed result (like a sum), while `RecursiveAction` is used for side-effect-only tasks (like sorting in place).

### 3.4 Fork-Join in Java — SumTask Implementation

*Figure 4.18 — Fork-join calculation using the Java API (textbook p.183)*

```java
import java.util.concurrent.*;

public class SumTask extends RecursiveTask<Integer> {
    static final int THRESHOLD = 1000;
    private int begin;
    private int end;
    private int[] array;

    public SumTask(int begin, int end, int[] array) {
        this.begin = begin;
        this.end = end;
        this.array = array;
    }

    protected Integer compute() {
        if (end - begin < THRESHOLD) {
            int sum = 0;
            for (int i = begin; i <= end; i++)
                sum += array[i];
            return sum;
        } else {
            int mid = (begin + end) / 2;
            SumTask leftTask = new SumTask(begin, mid, array);
            SumTask rightTask = new SumTask(mid + 1, end, array);
            leftTask.fork();
            rightTask.fork();
            return rightTask.join() + leftTask.join();
        }
    }
}
```

> **Code walkthrough:**
> 1. If the range is smaller than THRESHOLD (1000), compute directly — this is the **base case**
> 2. Otherwise, split the range in half, fork both halves as subtasks
> 3. `fork()` submits the subtask to the ForkJoinPool for parallel execution
> 4. `join()` blocks until the subtask completes and returns its result
> 5. Results are combined with `+`
>
> This looks exactly like a recursive merge sort, except that each recursive call runs as a **separate task** that the pool can execute on a different thread.

### 3.5 Work Stealing

**Work Stealing algorithm**:
- Each thread maintains its own **task queue**
- When its own queue is empty → **steals tasks from another thread's queue**
- Efficiently handles thousands of tasks with just a few worker threads

```text
Thread 1: [Task A] [Task B] [Task C]
Thread 2: [Task D]  (empty)  (empty) ← steals Task C from Thread 1
Thread 3: [Task E] [Task F]  (empty)
Thread 4:  (empty)  (empty)  (empty) ← steals Task F from Thread 3
```

**Determining THRESHOLD**:
- The criterion for deciding whether a problem is "small enough"
- In the textbook example, THRESHOLD = 1000
- In practice, the optimal value should be determined through **timing experiments**

> **Key Point:** Work stealing is what makes fork-join efficient in practice. Without it, if tasks are unevenly distributed, some threads would sit idle while others are overloaded. Work stealing uses a **double-ended queue (deque)** — the owning thread pushes/pops from one end, while thieves steal from the other end, minimizing contention.

> **[Algorithms]** The work-stealing strategy achieves **load balancing** without a central scheduler. Each thread acts independently: do your own work first, then steal from others. This decentralized approach scales well because it avoids the bottleneck of a single task queue shared by all threads.

---

<br>

## 4. OpenMP

### 4.1 OpenMP Overview

**OpenMP (Open Multi-Processing)**:
- A parallel programming API based on **compiler directives** for C, C++, and Fortran
- Supports parallel programming in **shared-memory** environments
- Developers mark **parallel regions** with directives
- The OpenMP runtime library executes those regions in parallel

**Basic usage**:

```c
#include <omp.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    /* sequential code */

    #pragma omp parallel
    {
        printf("I am a parallel region.");
    }

    /* sequential code */
    return 0;
}
```

`#pragma omp parallel` → **automatically creates as many threads as there are processing cores**
- Dual-core → 2 threads, Quad-core → 4 threads

> **`#pragma`** is a C/C++ compiler directive that provides implementation-specific instructions. The OpenMP directives (`#pragma omp ...`) are ignored by compilers that don't support OpenMP, so the code remains valid C — it just runs sequentially. This is a major advantage: **the same code works both with and without OpenMP**, making it easy to add parallelism to existing serial code.

### 4.2 parallel for

**Automatic parallelization of for loops**: `#pragma omp parallel for`

```c
#pragma omp parallel for
for (i = 0; i < N; i++) {
    c[i] = a[i] + b[i];
}
```

- **Automatically partitions** the operation of adding elements of arrays `a` and `b` into `c`
- OpenMP divides the iteration space among as many threads as available
- Example: N=1000, 4 threads → each thread handles 250 iterations

**Execution model**:

```text
Sequential    #pragma omp parallel for     Sequential
   code    ─→  ┌─ Thread 0: i=0..249  ─┐ ─→  code
               │  Thread 1: i=250..499  │
               │  Thread 2: i=500..749  │
               └─ Thread 3: i=750..999 ─┘
                    (implicit barrier)
```

> **Implicit barrier**: At the end of a `parallel for` region, all threads synchronize — no thread proceeds past the barrier until every thread has finished its iterations. This ensures the sequential code after the parallel region sees complete results.

> **[Computer Architecture]** The `parallel for` pattern maps directly to **data parallelism** from Week 4. Each thread performs the *same operation* (`c[i] = a[i] + b[i]`) on a *different subset of data*. This is also the model used by GPU computing (CUDA's `<<<blocks, threads>>>` syntax achieves similar partitioning).

### 4.3 Additional Features

**Manually setting the number of threads**:

```c
#include <omp.h>
#include <stdio.h>

int main() {
    omp_set_num_threads(4);  // set number of threads to 4

    #pragma omp parallel
    {
        printf("Thread %d of %d\n",
            omp_get_thread_num(),
            omp_get_num_threads());
    }

    return 0;
}
```

**Data sharing attributes**:
- `shared(var)` — variable shared by all threads
- `private(var)` — each thread gets its own copy of the variable
- `reduction(+:sum)` — combines each thread's result using the specified operation

**Compilation**: `gcc -fopenmp program.c -o program`

> **Data sharing attributes are critical for correctness.** Without them, all variables are `shared` by default in a `parallel` region (except for the loop counter in `parallel for`, which is `private` by default). A common bug is accidentally sharing a variable that should be private — this causes race conditions just like the `global_sum` example from Week 4's lab.

### 4.4 reduction Example

**Parallelizing array summation with reduction**:

```c
#include <omp.h>
#include <stdio.h>

int main() {
    int sum = 0;
    int a[1000];

    // Initialize array
    for (int i = 0; i < 1000; i++)
        a[i] = i + 1;

    // reduction: automatically sums partial results from each thread
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; i++) {
        sum += a[i];
    }

    printf("Sum = %d\n", sum);  // 500500
    return 0;
}
```

- Each thread maintains a **private copy** of `sum`
- At the end of the parallel region, all threads' sums are **combined using the + operation**
- Enables safe summation without race conditions

> Compare this with the Week 4 lab's race condition on `global_sum`. In that lab, `global_sum += array[i]` caused data loss because multiple threads performed non-atomic read-modify-write on the same variable. OpenMP's `reduction` clause solves this elegantly: each thread gets its own private copy, and the runtime merges them safely at the end. No locks, no race conditions, one line of code.

---

<br>

## 5. Grand Central Dispatch (GCD)

### 5.1 GCD Overview

**GCD (Grand Central Dispatch)**:
- Technology developed by Apple for macOS/iOS
- A combination of **runtime library + API + language extensions**
- Similar to OpenMP, automatically manages threading details

**Core mechanism**: **Dispatch Queue**
- Tasks placed in a dispatch queue are executed by GCD, which **assigns threads from a thread pool**

**Two types of Dispatch Queues**:

| Type | Description |
|------|-------------|
| **Serial Queue** | Dequeues in FIFO order, executes only one task at a time |
| **Concurrent Queue** | Dequeues in FIFO order, executes multiple tasks simultaneously |

> **Key distinction**: Both queue types dequeue in FIFO order, but a serial queue waits for the current task to finish before starting the next, while a concurrent queue can start a new task even if previous tasks haven't finished. Think of it as: serial = one cashier line, concurrent = multiple cashier lines all pulling from the same waiting area.

### 5.2 Serial vs Concurrent Queue

**Serial Queue** (= Private Dispatch Queue):
- Each process has its own serial queue (**main queue**)
- Developers can create additional serial queues
- Useful for **tasks requiring sequential execution**

**Concurrent Queue** (= Global Dispatch Queue):
- System-wide queues
- Four **QoS (Quality of Service)** classes:

| QoS Class | Description | Example |
|-----------|-------------|---------|
| `QOS_CLASS_USER_INTERACTIVE` | UI/event handling, small tasks | Animations |
| `QOS_CLASS_USER_INITIATED` | User is waiting for results | Opening file/URL |
| `QOS_CLASS_UTILITY` | Long-running tasks, immediate result not needed | Data import |
| `QOS_CLASS_BACKGROUND` | Tasks invisible to the user | Indexing, backup |

> **[Mobile Development]** QoS classes are crucial for iOS/macOS development. The system uses these priorities to allocate CPU time and manage power consumption. A common mistake is running network requests on `USER_INTERACTIVE` — this degrades UI responsiveness because the system over-prioritizes network I/O at the expense of animations and touch handling. Use `USER_INITIATED` for user-triggered network requests and `UTILITY` or `BACKGROUND` for prefetching.

### 5.3 Blocks and Closures

**In C/C++/Objective-C: Block** — defines a self-contained unit of work using `^{ }` syntax

```c
// block definition
^{ printf("I am a block"); }

// create a serial queue and submit work
dispatch_queue_t q =
    dispatch_queue_create("my.queue", NULL);

// use concurrent global queue
dispatch_queue_t gq =
    dispatch_get_global_queue(
        DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

// submit asynchronous work
dispatch_async(gq, ^{
    printf("Running in background\n");
});
```

**In Swift: Closure** — same concept as Block, used without `^`

```text
let queue = dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0)
dispatch_async(queue, { print("I am a closure.") })
```

> Blocks and closures are the "task" units in GCD. Just as OpenMP uses `#pragma` directives to mark parallel regions, GCD uses blocks/closures to define units of work. The key difference is that GCD tasks are submitted to **queues** (async or sync), giving the developer more control over execution order and concurrency.

### 5.4 GCD Internal Architecture

**GCD's Thread Pool Management**:
- Internally composed of **POSIX threads**
- GCD **dynamically adjusts** the pool size (expand/shrink)
- Automatically managed based on application demand and system capacity

**libdispatch library**:
- The implementation of GCD
- Open-sourced by Apple under the **Apache Commons License**
- Ported to other operating systems such as FreeBSD

```text
┌──────────────────────────────────────────┐
│  Application  (blocks / closures)        │
├──────────────────────────────────────────┤
│  Dispatch Queues (serial / concurrent)   │
├──────────────────────────────────────────┤
│  libdispatch (GCD runtime)               │
│  - Dynamic thread pool management        │
│  - QoS-based scheduling                  │
├──────────────────────────────────────────┤
│  POSIX Threads (pthreads)                │
├──────────────────────────────────────────┤
│  Kernel Threads                          │
└──────────────────────────────────────────┘
```

> This layered architecture shows how GCD abstracts threading all the way from application code down to kernel threads. The developer writes blocks/closures at the top, and 4 layers of abstraction handle everything else. Notice that GCD ultimately sits on top of POSIX threads — even Apple's "magic" threading is built on the same Pthreads we studied in Week 4.

---

<br>

## 6. Intel Thread Building Blocks (TBB)

### 6.1 Intel TBB Overview

**Intel TBB (Threading Building Blocks)**:
- A **C++ template library** for designing parallel applications
- **No special compiler or language extensions required** (works with library alone)
- Developers **identify parallelizable tasks**, and the TBB task scheduler maps them to threads

**TBB Task Scheduler characteristics**:
- **Load balancing**: distributes workload evenly among threads
- **Cache aware**: prioritizes tasks that are likely to have data in cache

**Key features provided**:
- Parallel loop constructs (`parallel_for`, `parallel_reduce`)
- Atomic operations
- Mutual exclusion locking
- Concurrent data structures (hash map, queue, vector)

> **Comparison with OpenMP**: OpenMP uses **compiler directives** (`#pragma`), so it requires a compiler that supports OpenMP. TBB uses pure **C++ templates and libraries**, so any standard C++ compiler works. This makes TBB more portable in some environments, but OpenMP is simpler syntactically.

### 6.2 parallel_for Example

**Serial code**:

```cpp
for (int i = 0; i < n; i++) {
    apply(v[i]);
}
```

**Converted to TBB `parallel_for`**:

```cpp
#include <tbb/tbb.h>
using namespace tbb;

parallel_for(size_t(0), n, [=](size_t i) {
    apply(v[i]);
});
```

**How it works**:
- First two parameters: iteration space (0 to n-1)
- Third parameter: **C++ lambda function** `[=](size_t i)` → executed for each `i`
- TBB splits loop iterations into **chunks**
- Creates multiple tasks and assigns them to available threads
- Similar approach to Java's Fork-Join

> The `[=]` in the lambda captures all variables by value. This is important for thread safety — each thread works with its own copy of captured variables. If you need to modify shared state, you'd use `[&]` (capture by reference) with appropriate synchronization.

### 6.3 parallel_reduce Example

**Implementing array summation with `parallel_reduce`**:

```cpp
#include <tbb/tbb.h>
using namespace tbb;

// Using parallel_reduce
int sum = parallel_reduce(
    blocked_range<int>(0, n),  // iteration space
    0,                          // initial value
    [&](const blocked_range<int>& r, int local_sum) {
        for (int i = r.begin(); i < r.end(); i++)
            local_sum += data[i];
        return local_sum;
    },
    [](int x, int y) { return x + y; }  // combine function
);
```

- Task partitioning and scheduling are **automatically handled by the library**
- Maintains load balance using the **work-stealing** algorithm
- Available in both commercial and open-source versions (Windows, Linux, macOS)

> **Comparison with OpenMP reduction**: Both achieve the same goal (parallel array summation), but the approach differs:
> - OpenMP: `#pragma omp parallel for reduction(+:sum)` — one line of directive
> - TBB: `parallel_reduce(range, init, body, combine)` — explicit function call with lambda
>
> OpenMP is more concise; TBB is more flexible (you can define arbitrary combine operations, not just built-in operators).

---

<br>

## 7. Implicit Threading — Comparison of 5 Techniques

| Technique | Language/Platform | Approach | Key Feature |
|-----------|------------------|----------|-------------|
| **Thread Pool** | Java, Windows, Android | Pre-create threads in pool | General-purpose, reuse |
| **Fork-Join** | Java 1.7+ | Divide-and-conquer | Work stealing |
| **OpenMP** | C/C++/Fortran | Compiler directives | Simplest syntax |
| **GCD** | macOS/iOS (C, Swift) | Dispatch queue | QoS-based |
| **Intel TBB** | C++ | Template library | Cache aware |

**Common traits**:
- Developers only **identify tasks (parallel work units)**
- Thread creation/management/scheduling is **handled by the library/runtime**
- Improves program correctness and reduces development complexity

> **Exam Tip:** You should be able to name all five techniques and their distinguishing feature. A helpful mnemonic: **T-F-O-G-I** (Thread pool, Fork-join, OpenMP, GCD, Intel TBB). Remember that all five share the core idea: **developers define tasks, the system manages threads**.

---

<br>

## 8. Threading Issues

### 8.1 Threading Issues Overview

Five key issues to consider in multithreaded programming:

1. **fork() and exec() semantics** — duplicating a multithreaded process
2. **Signal Handling** — delivering signals in a multithreaded environment
3. **Thread Cancellation** — canceling a running thread
4. **Thread-Local Storage (TLS)** — per-thread unique data
5. **Scheduler Activations** — communication between kernel and thread library

> These issues arise from the fundamental tension in multithreading: threads share the process's resources (which makes them efficient) but sometimes need independent behavior (which creates complexity). Each issue below is a specific case where the shared-vs-independent boundary causes problems.

### 8.2 fork() and exec() Semantics

**Problem**: What happens when one thread in a multithreaded process calls `fork()`?

**Two possible behaviors**:
1. **Duplicate all threads** — new process has all threads from the parent
2. **Duplicate only the calling thread** — new process is single-threaded

```text
  Parent Process                  Child Process (Option 1)
  ┌─────────────┐                ┌─────────────┐
  │ Thread 1    │   fork()       │ Thread 1'   │
  │ Thread 2 ───┼──────────→    │ Thread 2'   │  All threads duplicated
  │ Thread 3    │                │ Thread 3'   │
  └─────────────┘                └─────────────┘

  Parent Process                  Child Process (Option 2)
  ┌─────────────┐                ┌─────────────┐
  │ Thread 1    │   fork()       │             │
  │ Thread 2 ───┼──────────→    │ Thread 2'   │  Only calling thread duplicated
  │ Thread 3    │                │             │
  └─────────────┘                └─────────────┘
```

**Selection criteria:**

| Scenario | fork() behavior | Reason |
|----------|----------------|--------|
| exec() called immediately after fork() | Duplicate only the calling thread | Since exec() replaces the entire process, duplicating other threads is wasteful |
| Running without exec() after fork() | Duplicate all threads | The child process also needs all threads |

**When exec() is called**:
- **Replaces** the entire process (**including all threads**) with a new program
- Therefore, in the fork() + exec() pattern, duplicating only the calling thread is sufficient

**Some UNIX systems**: Provide both versions of fork()

> **Key Point:** In practice, `fork()` followed immediately by `exec()` is the most common pattern (Week 2-W03). In this case, duplicating all threads would be wasteful because `exec()` discards the entire address space anyway. POSIX's `fork()` duplicates only the calling thread, while some systems provide `forkall()` for duplicating all threads.

> **Caution:** When fork() duplicates only the calling thread, any mutexes held by other threads remain locked in the child process, but the threads that would unlock them don't exist. This can lead to **deadlocks** in the child process — another reason why fork()+exec() is preferred.

### 8.3 Signal Handling — Concepts and Patterns

**Signal**: A mechanism in UNIX systems to notify a process that a specific event has occurred

**Three-step pattern followed by all signals**:
1. A signal is **generated** by a specific event
2. The signal is **delivered** to the process
3. The delivered signal is **handled**

**Types of signal handlers**:
1. **Default signal handler**: default handler executed by the kernel
2. **User-defined signal handler**: custom handler defined by the programmer

| Category | Synchronous Signal | Asynchronous Signal |
|----------|-------------------|-------------------|
| **Cause** | Triggered by the process itself | Generated externally |
| **Delivered to** | The process that performed the action | Delivered to another process |
| **Examples** | Illegal memory access, div by 0 | `Ctrl+C`, timer expire |

> **Synchronous signals** are the result of a thread's own actions — like dividing by zero or accessing invalid memory. The thread "caused" the signal, so delivery is straightforward. **Asynchronous signals** come from outside the process (the user pressing Ctrl+C, a timer expiring, another process sending a signal), which is where the multithreading complication arises: *which* thread should receive it?

### 8.4 Signal Handling — Multithreaded Environment

**Single-threaded**: Signals are always delivered to that process (simple)

**Multi-threaded**: Where should the signal be delivered?

**Four options** (textbook p.189):
1. Deliver to the **thread that caused the signal** (suitable for synchronous signals)
2. Deliver to **every thread in the process** (e.g., `Ctrl+C`)
3. Deliver to **certain threads only** (threads that have not blocked the signal)
4. Designate a **dedicated signal handler thread**

```c
// UNIX: deliver signal to a specific process
kill(pid_t pid, int signal);

// Pthreads: deliver signal to a specific thread
pthread_kill(pthread_t tid, int signal);
```

**POSIX**: Each thread can specify which signals to accept/block
- Asynchronous signal → delivered to the **first thread** that does not block it
- **Windows**: Uses **APC (Asynchronous Procedure Call)** instead of signals — delivered directly to a specific thread

> **Exam Tip:** The four signal delivery options are frequently tested. Remember them with: (1) the causer, (2) everyone, (3) some (unblocked), (4) one dedicated thread. In practice, option 4 (dedicated signal handler thread) is a common pattern in server applications — all other threads block signals, and one designated thread handles them in a controlled loop.

### 8.5 Thread Cancellation — Concept

**Thread Cancellation**: Terminating a thread **before it has completed**

**Usage examples**:
- Multiple threads searching a DB → when one finds the result, **cancel the remaining threads**
- Pressing the Stop button in a web browser → **cancel all** image loading threads

The thread to be canceled is called the **target thread**.

**Two cancellation approaches**:

| Approach | Description | Safety |
|----------|-------------|--------|
| **Asynchronous cancellation** | Immediate termination | Dangerous |
| **Deferred cancellation** | Terminates at a cancellation point | Safe |

### 8.6 Asynchronous vs Deferred Cancellation

**Problems with Asynchronous Cancellation**:
- Immediate termination while resources are allocated → **resource leak**
- Cancellation during shared data update → **data inconsistency**
- The OS reclaims system resources but **cannot reclaim all resources**
- Pthreads documentation **discourages its use**

**Deferred Cancellation**:
- Only sends a "cancellation request" to the thread
- Actual termination occurs only when the target thread reaches a **cancellation point**
- Enables safe resource cleanup

```text
Asynchronous:  cancel ──→ [Immediate termination!] (no resource cleanup)

Deferred:      cancel ──→ [continues running] ──→ [cancellation point] ──→ [cleanup then terminate]
```

> Think of asynchronous cancellation like pulling the power plug on a computer — fast, but you risk data corruption and resource leaks. Deferred cancellation is like clicking "Shut Down" — the system finishes critical operations, saves state, releases resources, then terminates cleanly.

### 8.7 Thread Cancellation — Pthreads API

**Three cancellation modes in Pthreads**:

| Mode | State | Type |
|------|-------|------|
| **Off** | Disabled | -- |
| **Deferred** (default) | Enabled | Deferred |
| **Asynchronous** | Enabled | Asynchronous |

```c
pthread_t tid;

/* create the thread */
pthread_create(&tid, 0, worker, NULL);

/* cancel the thread */
pthread_cancel(tid);

/* wait for the thread to terminate */
pthread_join(tid, NULL);
```

- `pthread_cancel(tid)` — only sends a cancellation **request** (does not terminate immediately!)
- Actual cancellation is determined by the target thread's cancellation mode
- If cancellation is **disabled** → the request remains in **pending** state

> **Important distinction**: `pthread_cancel()` is a **request**, not a command. The target thread decides when (or if) to honor it. This is fundamentally different from `kill()`, which delivers a signal that typically terminates the process immediately. The deferred model gives the target thread control over its own termination.

### 8.8 Cancellation Points and Cleanup Handlers

**Cancellation Point**:
- In deferred cancellation, a point where the thread "can be canceled"
- Most **blocking system calls** in POSIX/standard C library are cancellation points
  - Examples: `read()`, `write()`, `sleep()`, `pthread_join()`, etc.
- The full list can be checked with `man pthreads`

```c
while (1) {
    /* do some work for awhile */
    ...

    /* check if there is a cancellation request */
    pthread_testcancel();
}
```

**`pthread_testcancel()`**:
- Explicitly creates a cancellation point
- If a pending cancellation request exists → thread terminates
- If none → returns normally, execution continues

**Cleanup Handler**: A function that safely releases resources (memory, files, locks, etc.) when a thread is canceled

> **Practical pattern**: Long-running worker threads that don't call blocking system calls should periodically call `pthread_testcancel()`. Without this, a deferred cancellation request would remain pending forever because there's no cancellation point. The thread would appear to "ignore" the cancel request.

### 8.9 Thread Cancellation — Java

Java uses a policy **similar to Pthreads' deferred cancellation**:

```java
Thread worker;
...

/* set the interruption status of the thread */
worker.interrupt();
```

`interrupt()`: Sets the target thread's **interruption status to true**

```java
while (!Thread.currentThread().isInterrupted()) {
    // ... do work ...
}
```

**`isInterrupted()`**: Returns the thread's interruption status as a boolean

**Comparison with Pthreads**:

| | Pthreads | Java |
|--|---------|------|
| Cancellation request | `pthread_cancel(tid)` | `worker.interrupt()` |
| Status check | `pthread_testcancel()` | `isInterrupted()` |
| Automatic cancellation points | Blocking system calls | Blocking methods (sleep, wait, join) |

**Note**: On Linux, Pthreads cancellation is internally implemented using **signals**

> Java's approach is even safer than Pthreads: `interrupt()` only sets a boolean flag. The thread must explicitly check `isInterrupted()` or be in a blocking method that throws `InterruptedException`. This makes it impossible to accidentally terminate a thread at an unsafe point.

### 8.10 Thread-Local Storage (TLS)

**TLS (Thread-Local Storage)**: A mechanism where each thread maintains its own **copy of data**

**Why is it needed?**
- Threads fundamentally **share** process data
- However, sometimes threads need **their own unique data**
- Example: Assigning a **unique transaction ID** to each thread in a transaction processing system

**TLS vs Local Variables**:

| | Local Variables | TLS |
|--|----------------|-----|
| Lifetime | Only during the function call | **Throughout the thread's entire lifetime** |
| Visibility | Within the function scope | Accessible from all functions |
| Similarity | Allocated on the stack | Similar to **static data** (but unique per thread) |

> Think of TLS as each thread having its own "locker" at the office. Everyone shares the same open workspace (shared data), but each person has a private locker (TLS) that persists throughout their employment (thread lifetime) and only they can access. Local variables are more like sticky notes you use during a meeting — they're discarded when the meeting ends.

### 8.11 TLS — Implementation by Language

**TLS support across languages/compilers**:

```c
// GCC: __thread keyword
static __thread int threadID;

// C11 standard: _Thread_local keyword
_Thread_local int var = 0;
```

```java
// Java: ThreadLocal<T> class
ThreadLocal<Integer> value =
    ThreadLocal.withInitial(() -> 0);

value.set(42);           // set value for the current thread
int v = value.get();     // get the current thread's value
```

```csharp
// C#: [ThreadStatic] attribute
[ThreadStatic]
static int threadLocalVar;
```

**Especially useful in thread pool environments**: Each worker thread can maintain unique state even when thread creation is not directly controlled by the developer
- Pthreads: Uses `pthread_key_t` type to create/query/delete TLS keys

> **[Programming Languages]** TLS is implemented differently under the hood depending on the platform. On Linux/x86-64, TLS data is accessed through the `fs` segment register, which points to each thread's own TLS block. This means TLS access is nearly as fast as accessing a regular global variable — there's no lock or hash table lookup involved.

### 8.12 Scheduler Activations — LWP and Upcalls

**Problem**: In Many-to-Many / Two-Level models, **communication** between the kernel and thread library is needed

**LWP (Lightweight Process)**:
- An **intermediate data structure** between user threads and kernel threads
- Appears as a **virtual processor** to the user-level thread library
- Each LWP is **attached to one kernel thread**

*Silberschatz, Figure 4.20 — Lightweight process (LWP)*

**Number of LWPs**: A CPU-bound application needs only 1, while an I/O-intensive application needs as many as the number of concurrent blocking I/O operations

> The LWP concept was introduced in Week 4 as part of the Two-Level Model. Here we see it in its operational context: the LWP is the "communication channel" between the kernel and the user-level thread library. When the kernel needs to inform the thread library about an event (like a thread blocking), it does so through the LWP.

### 8.13 Scheduler Activations — Upcall Mechanism

**Upcall**: An **event notification** from the kernel to the user-level thread library

**Workflow** (when a thread blocks):

```text
1. Thread A blocks on an I/O request
   ┌──────────────────────────────────────┐
   │ Kernel: "Thread A is blocking"       │ ──→ Upcall
   └──────────────────────────────────────┘

2. Kernel allocates a new virtual processor (LWP)

3. Thread library executes the upcall handler:
   - Saves the state of the blocking thread
   - Schedules another eligible thread on the new LWP

4. When the blocking event completes:
   ┌──────────────────────────────────────┐
   │ Kernel: "Thread A is ready"          │ ──→ Upcall
   └──────────────────────────────────────┘

5. Thread library transitions Thread A back to runnable state
```

> **Upcall vs System Call**: A system call is a **downward** request (user → kernel): "please do I/O." An upcall is an **upward** notification (kernel → user thread library): "your thread just blocked." This bidirectional communication is what makes the Many-to-Many model work — without upcalls, the thread library wouldn't know when a user thread blocked in the kernel, and it wouldn't be able to schedule another thread in its place.

> **Note:** Scheduler activations are primarily relevant to Many-to-Many models. Since most modern systems use the One-to-One model (Linux, Windows), scheduler activations are less commonly encountered in practice today. However, the concept remains important for understanding how hybrid threading models work.

---

<br>

## 9. Operating-System Examples

### 9.1 Windows Threads — Overview

**Windows Thread implementation**:
- Uses the **One-to-One model** (each user-level thread → kernel thread)
- Threads are created with the Windows API's `CreateThread()`

**General components of a thread** (the thread's **context**):
- Thread ID (unique identifier), Register set (processor state), Program counter
- **User stack** (for user mode) + **Kernel stack** (for kernel mode)
- Private storage area (used by run-time libraries, DLLs)

**Three key data structures** (Figure 4.21):

| Structure | Location | Key Content |
|-----------|----------|------------|
| **ETHREAD** | Kernel space | Thread start address, parent process pointer, KTHREAD pointer |
| **KTHREAD** | Kernel space | Scheduling/synchronization info, kernel stack, TEB pointer |
| **TEB** | User space | Thread ID, user-mode stack, TLS array |

> **ETHREAD** = Executive Thread, **KTHREAD** = Kernel Thread, **TEB** = Thread Environment Block. The naming follows Windows' layered kernel architecture: the Executive layer (ETHREAD) handles high-level thread management, the Kernel layer (KTHREAD) handles scheduling, and the TEB provides user-space accessible information.

### 9.2 Windows Threads — Data Structures

*Silberschatz, Figure 4.21 — Windows thread data structures*

- ETHREAD, KTHREAD → **accessible only by the kernel** (kernel space)
- TEB → **accessible in user mode** (user space)
- Connection: ETHREAD → KTHREAD → TEB

```text
  Kernel Space                          User Space
  ┌─────────────┐                      ┌─────────────┐
  │  ETHREAD    │                      │    TEB      │
  │ - start addr│    ┌─────────────┐   │ - thread ID │
  │ - process   │──→ │  KTHREAD    │──→│ - user stack│
  │   pointer   │    │ - scheduling│   │ - TLS array │
  └─────────────┘    │ - kernel    │   └─────────────┘
                     │   stack     │
                     └─────────────┘
```

> **Exam Tip:** "ETHREAD → KTHREAD → TEB" is the key chain to remember. ETHREAD and KTHREAD are in kernel space (protected), while TEB is in user space (accessible to the application). The TLS array in the TEB is how Windows implements Thread-Local Storage at the OS level.

### 9.3 Linux Threads — Task Concept

**Linux's unique approach**:
- **Does not distinguish** between process and thread
- All execution flows are called **"tasks"**
- Each task has its own unique **struct task_struct**

**Key point**: `task_struct` does not store data directly but contains **pointers to other data structures**

```text
  ┌─ task_struct ──────────────────────────┐
  │                                         │
  │  *files  ──→ [open file table]          │
  │  *signal ──→ [signal handling info]     │
  │  *mm     ──→ [memory descriptors]       │
  │  *fs     ──→ [filesystem info]          │
  │  pid, state, priority, ...              │
  │                                         │
  └─────────────────────────────────────────┘
```

> **Key Point:** Linux's design is elegantly simple: a "process" and a "thread" are both just a `task_struct`. The difference between them is **how much they share**. A new process (via `fork()`) gets its own copies of everything. A new thread (via `clone()` with sharing flags) shares pointers to the parent's data structures. This unified model avoids maintaining separate code paths for process and thread management.

### 9.4 Linux — clone() System Call

**`clone()`**: The system call for creating tasks in Linux
- **Flags** determine the **level of sharing** between parent and child

**Usage pattern**:

```text
fork() = clone(no flags)
  → Nothing shared (complete copy)

thread = clone(CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND)
  → Almost everything shared (= thread creation)
```

*Silberschatz, Figure 4.22 — clone() flags*

Key clone flags:

| Flag | What is shared |
|------|---------------|
| `CLONE_VM` | Virtual memory (address space) |
| `CLONE_FS` | Filesystem information (root dir, cwd) |
| `CLONE_FILES` | Open file descriptors |
| `CLONE_SIGHAND` | Signal handlers |

> When `pthread_create()` is called on Linux, it internally calls `clone()` with all sharing flags set. This is why Pthreads on Linux creates threads that share the address space, file descriptors, and signal handlers — it's all controlled by the `clone()` flags.

### 9.5 Linux — fork() vs clone() Comparison

**When fork() is called**: New task + **copies** all of the parent's data structures

**When clone() is called (with sharing flags)**: New task + **shares** the parent's data structures via pointers

```text
  fork() — No sharing                clone() — With sharing flags
  ┌──────────┐  ┌──────────┐        ┌──────────┐  ┌──────────┐
  │ Parent   │  │ Child    │        │ Parent   │  │ Child    │
  │ task_    │  │ task_    │        │ task_    │  │ task_    │
  │ struct   │  │ struct   │        │ struct   │  │ struct   │
  │          │  │          │        │          │  │          │
  │ *mm ─→[A]│  │ *mm ─→[A']│      │ *mm ──┐  │  │ *mm ──┐  │
  │ *files→[B]│  │*files→[B']│      │ *files─┤  │  │*files─┤  │
  └──────────┘  └──────────┘        └───────┼──┘  └──────┼──┘
   Different copies                         └──→ [SHARED] ←┘
                                           Same data structures shared
```

**Extension of clone() — Containers**: By using specific flags to isolate namespaces, containers (Docker, LXC) can be created (Ch 18)

> **[Systems Programming]** Linux's `clone()` is the fundamental building block for:
> - **Threads**: sharing everything (`CLONE_VM | CLONE_FILES | ...`)
> - **Processes**: sharing nothing (plain `fork()`)
> - **Containers**: selective sharing + namespace isolation (`CLONE_NEWNS`, `CLONE_NEWPID`, etc.)
>
> This spectrum — from full isolation (processes) to full sharing (threads) to selective isolation (containers) — is all controlled by the same system call. It's one of Linux's most elegant design decisions.

---

<br>

## 10. Lab — OpenMP Parallel Programming

### 10.1 Lab: OpenMP Matrix Multiplication Parallelization

```c
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 1000

double A[N][N], B[N][N], C[N][N];

void multiply_parallel(int num_threads) {
    omp_set_num_threads(num_threads);
    double start = omp_get_wtime();

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < N; k++)
                C[i][j] += A[i][k] * B[k][j];
        }

    double end = omp_get_wtime();
    printf("Threads: %d, Time: %.3f sec\n",
           num_threads, end - start);
}
```

Compile: `gcc -fopenmp matrix_mul.c -o matrix_mul`

> **`collapse(2)`** merges the outer two loops (`i` and `j`) into a single iteration space of N*N = 1,000,000 iterations, which are then distributed among threads. Without `collapse`, only the outermost loop (`i`) would be parallelized (1,000 iterations). More iterations per thread generally improves load balancing.

> **Note**: The innermost `k` loop is **not** parallelized because `C[i][j]` accumulates a sum — parallelizing it would require a reduction. The outer loops are independent (different `(i,j)` pairs write to different `C[i][j]` elements), making them safe to parallelize without synchronization.

### 10.2 Lab: Performance Comparison by Thread Count

```c
int main() {
    srand(time(NULL));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            A[i][j] = (double)rand() / RAND_MAX;
            B[i][j] = (double)rand() / RAND_MAX;
        }

    multiply_parallel(1);
    multiply_parallel(2);
    multiply_parallel(4);
    multiply_parallel(8);
    return 0;
}
```

**Things to observe in the experiment**:
- Change in execution time as the number of threads increases
- Comparison between Amdahl's Law and actual speedup
- Performance impact when creating more threads than the number of cores
- Cache effects and memory bandwidth bottleneck

> **[Computer Architecture]** Matrix multiplication is a classic benchmark for observing the gap between theoretical and actual speedup. Even with a purely parallel workload (S ≈ 0), you won't see linear speedup because of:
> 1. **Memory bandwidth**: All threads compete for the same memory bus
> 2. **Cache effects**: The `B[k][j]` access pattern has poor spatial locality (column-major access in a row-major array)
> 3. **Thread overhead**: Creating and synchronizing threads has a cost
>
> If more threads are created than physical cores, performance may actually **decrease** due to context switching overhead.

### 10.3 Lab: OpenMP reduction Lab

```c
#include <omp.h>
#include <stdio.h>
#define SIZE 10000000

int main() {
    double *arr = malloc(SIZE * sizeof(double));
    double sum = 0.0;

    // Initialize array
    for (int i = 0; i < SIZE; i++)
        arr[i] = 1.0 / (i + 1);

    // Sequential execution
    double start = omp_get_wtime();
    for (int i = 0; i < SIZE; i++)
        sum += arr[i];
    printf("Sequential sum: %.6f, Time: %.4f sec\n",
           sum, omp_get_wtime() - start);

    // Parallel execution (reduction)
    sum = 0.0;
    start = omp_get_wtime();
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < SIZE; i++)
        sum += arr[i];
    printf("Parallel   sum: %.6f, Time: %.4f sec\n",
           sum, omp_get_wtime() - start);

    free(arr);
    return 0;
}
```

> This lab computes the partial sum of the harmonic series: 1/1 + 1/2 + 1/3 + ... + 1/10000000. Both sequential and parallel versions should produce the **same result** (or very close — floating-point addition is not perfectly associative, so the order of operations can cause tiny differences). The key observation is the **time difference**: the parallel version should be significantly faster on a multicore machine.

> **Comparison with Week 4 Lab**: In the Week 4 lab, we manually partitioned an array across Pthreads and used a `partial_sum[]` array to avoid race conditions. Here, OpenMP's `reduction` clause does all of that automatically in a single directive. This is the power of implicit threading — the same parallelization that took ~50 lines of Pthreads code takes 1 line with OpenMP.

---

<br>

## Summary

| Concept | Key Summary |
|:--------|:-----------|
| Implicit Threading | Delegates thread management to compiler/runtime; developers only identify parallel tasks |
| Thread Pool | Pre-create threads for reuse; 3 benefits: speed, resource bound, separation |
| Fork-Join | Divide-and-conquer parallelism; work stealing for load balance |
| OpenMP | Compiler directives (`#pragma omp`); simplest syntax for C/C++/Fortran |
| GCD | Apple's dispatch queue system; QoS-based task scheduling |
| Intel TBB | C++ template library; cache-aware task scheduler |
| fork()/exec() in MT | fork() can duplicate all or just calling thread; exec() replaces entire process |
| Signal Handling | 4 delivery options in multithreaded environments; each thread can block/accept signals |
| Thread Cancellation | Deferred (safe, at cancellation points) vs Asynchronous (dangerous, immediate) |
| TLS | Per-thread unique data; persists for thread's lifetime; accessible from all functions |
| Scheduler Activations | LWP as virtual processor; upcalls for kernel-to-user-library communication |
| Windows Threads | One-to-One model; ETHREAD → KTHREAD → TEB data structure chain |
| Linux Threads | No process/thread distinction — all are "tasks"; clone() flags control sharing |
| Textbook Scope | Silberschatz Ch 4, Sections 4.5–4.7 |

---


<br>

## Self-Check Questions

1. What is implicit threading and why has it become necessary? Name the five techniques covered in this chapter.
2. Explain the three benefits of thread pools. How does a thread pool differ from creating a new thread per request?
3. What is work stealing in the Fork-Join model, and why is it important for performance?
4. Write a simple OpenMP program that parallelizes array summation using `reduction`. What happens without the `reduction` clause?
5. When a multithreaded process calls `fork()`, should all threads be duplicated or only the calling thread? Explain the reasoning for both cases.
6. What are the four options for delivering signals in a multithreaded environment?
7. Compare asynchronous and deferred thread cancellation. Why does Pthreads default to deferred cancellation?
8. What is Thread-Local Storage (TLS) and how does it differ from local variables?
9. Describe the three Windows thread data structures (ETHREAD, KTHREAD, TEB) and their locations.
10. How does Linux's `clone()` system call unify process and thread creation? What do the sharing flags control?

---
