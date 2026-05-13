# Week 9 Lecture — Synchronization

> **Last Updated:** 2026-05-13
>
> Silberschatz, Operating System Concepts Ch 6 (Synchronization Tools), Ch 7 (Synchronization Examples)

> **Prerequisites**: Week 4–5 Threads & Concurrency. You should already understand processes, threads (user/kernel level), context switching, and shared memory.
>
> **Learning Objectives**:
> 1. Define a **race condition** and the three requirements of the **Critical-Section Problem** (Mutual Exclusion, Progress, Bounded Waiting)
> 2. Explain **Peterson's Solution** and why it can break on modern reordering hardware
> 3. Describe hardware support: **memory barriers**, **test_and_set**, **compare_and_swap (CAS)**, **atomic variables**
> 4. Use **mutex locks** and understand the spinlock vs sleeping-lock trade-off
> 5. Use **counting** and **binary semaphores** with and without a waiting queue
> 6. Reason about **monitors** and **condition variables** (wait/signal, signal-and-wait vs signal-and-continue)
> 7. Identify **liveness** failures: deadlock, priority inversion (and the priority-inheritance fix)
> 8. Solve the three classic problems: **Bounded-Buffer**, **Readers-Writers**, **Dining-Philosophers**
> 9. Map the abstract concepts to real systems: **Windows**, **Linux**, **POSIX**, **Java**

---

## Table of Contents

- [1. Background and the Critical-Section Problem](#1-background-and-the-critical-section-problem)
  - [1.1 Cooperating Processes and Data Inconsistency](#11-cooperating-processes-and-data-inconsistency)
  - [1.2 Race Condition](#12-race-condition)
  - [1.3 Race Conditions in the Kernel](#13-race-conditions-in-the-kernel)
  - [1.4 The Critical-Section Problem](#14-the-critical-section-problem)
  - [1.5 Preemptive vs Nonpreemptive Kernels](#15-preemptive-vs-nonpreemptive-kernels)
- [2. Peterson's Solution](#2-petersons-solution)
  - [2.1 Algorithm](#21-algorithm)
  - [2.2 Correctness Proof](#22-correctness-proof)
  - [2.3 Limitations on Modern Architectures](#23-limitations-on-modern-architectures)
- [3. Hardware Support for Synchronization](#3-hardware-support-for-synchronization)
  - [3.1 Memory Barriers (Memory Fences)](#31-memory-barriers-memory-fences)
  - [3.2 test_and_set](#32-test_and_set)
  - [3.3 compare_and_swap (CAS)](#33-compare_and_swap-cas)
  - [3.4 CAS — Ensuring Bounded Waiting](#34-cas--ensuring-bounded-waiting)
  - [3.5 Atomic Variables](#35-atomic-variables)
- [4. Mutex Locks](#4-mutex-locks)
  - [4.1 Mutex Lock — Basics](#41-mutex-lock--basics)
  - [4.2 Spinlocks and Busy Waiting](#42-spinlocks-and-busy-waiting)
- [5. Semaphores](#5-semaphores)
  - [5.1 Definition (wait / signal)](#51-definition-wait--signal)
  - [5.2 Semaphore Types and Usage](#52-semaphore-types-and-usage)
  - [5.3 Eliminating Busy Waiting (Waiting Queue)](#53-eliminating-busy-waiting-waiting-queue)
  - [5.4 Atomicity of Semaphore Implementation](#54-atomicity-of-semaphore-implementation)
- [6. Monitors](#6-monitors)
  - [6.1 Difficulty of Using Semaphores](#61-difficulty-of-using-semaphores)
  - [6.2 Monitor — Structure](#62-monitor--structure)
  - [6.3 Condition Variables](#63-condition-variables)
  - [6.4 Implementing a Monitor with Semaphores](#64-implementing-a-monitor-with-semaphores)
  - [6.5 ResourceAllocator Monitor Example](#65-resourceallocator-monitor-example)
- [7. Liveness](#7-liveness)
  - [7.1 Deadlock](#71-deadlock)
  - [7.2 Priority Inversion and Priority Inheritance](#72-priority-inversion-and-priority-inheritance)
  - [7.3 Evaluating Synchronization Tools](#73-evaluating-synchronization-tools)
- [8. Classic Problems of Synchronization](#8-classic-problems-of-synchronization)
  - [8.1 Bounded-Buffer Problem](#81-bounded-buffer-problem)
  - [8.2 Bounded-Buffer — Producer/Consumer Code](#82-bounded-buffer--producerconsumer-code)
  - [8.3 Readers-Writers Problem](#83-readers-writers-problem)
  - [8.4 Readers-Writers — First Problem Solution](#84-readers-writers--first-problem-solution)
  - [8.5 Reader-Writer Locks](#85-reader-writer-locks)
  - [8.6 Dining-Philosophers Problem](#86-dining-philosophers-problem)
  - [8.7 Dining-Philosophers — Semaphore Solution and Deadlock](#87-dining-philosophers--semaphore-solution-and-deadlock)
  - [8.8 Dining-Philosophers — Deadlock-Avoidance Strategies](#88-dining-philosophers--deadlock-avoidance-strategies)
  - [8.9 Dining-Philosophers — Monitor Solution](#89-dining-philosophers--monitor-solution)
- [9. Synchronization in Real Systems](#9-synchronization-in-real-systems)
  - [9.1 Windows Synchronization](#91-windows-synchronization)
  - [9.2 Linux Synchronization](#92-linux-synchronization)
  - [9.3 POSIX Mutex Locks](#93-posix-mutex-locks)
  - [9.4 POSIX Semaphores (Named and Unnamed)](#94-posix-semaphores-named-and-unnamed)
  - [9.5 POSIX Condition Variables](#95-posix-condition-variables)
- [10. Java Synchronization](#10-java-synchronization)
  - [10.1 Java Monitors — `synchronized`, `wait()`/`notify()`](#101-java-monitors--synchronized-waitnotify)
  - [10.2 How Java Monitors Work Internally](#102-how-java-monitors-work-internally)
  - [10.3 ReentrantLock and Semaphore](#103-reentrantlock-and-semaphore)
  - [10.4 Java Condition Variables](#104-java-condition-variables)
- [11. Lab — Dining-Philosophers Implementation](#11-lab--dining-philosophers-implementation)
  - [11.1 Lab: Naive pthread Implementation](#111-lab-naive-pthread-implementation)
  - [11.2 Lab: Asymmetric Solution](#112-lab-asymmetric-solution)
  - [11.3 Lab: Monitor Solution with POSIX Condition Variables](#113-lab-monitor-solution-with-posix-condition-variables)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. Background and the Critical-Section Problem

### 1.1 Cooperating Processes and Data Inconsistency

A **cooperating process** is one that can affect, or be affected by, the execution of other processes. Cooperation requires **shared state**, which can be exchanged through:

- **Shared memory** — multiple processes map the same memory region.
- **Message passing** — processes exchange messages explicitly.
- **Logical address space sharing** — multiple threads in a single process all share the same address space.

The fundamental problem: **concurrent access to shared data can produce inconsistent results**. Without a discipline that orders these accesses, the final state of the shared data depends on the unpredictable interleaving of instructions.

> **Why this is the foundation of the whole chapter**: Every synchronization tool you will see — locks, semaphores, monitors — exists for one reason: to impose **orderly execution** on concurrent accesses to shared data, so that correctness no longer depends on luck.

### 1.2 Race Condition

A **race condition** occurs when two or more processes (or threads) access shared data concurrently, and the **final result depends on the order in which the accesses interleave**.

**Classic example.** A producer thread executes `count++` while a consumer executes `count--`, with the initial value `count = 5`. Both `++` and `--` decompose into **load → modify → store**, and an unfortunate interleaving can lose one of the updates:

```text
T0: producer  register1 = count        {register1 = 5}
T1: producer  register1 = register1+1  {register1 = 6}
T2: consumer  register2 = count        {register2 = 5}
T3: consumer  register2 = register2-1  {register2 = 4}
T4: producer  count = register1        {count = 6}
T5: consumer  count = register2        {count = 4}  ← incorrect!
```

The correct final value is `count = 5` (one increment + one decrement). Depending on interleaving, the result can become **4** (consumer wins) or **6** (producer wins) — an off-by-one bug whose appearance is timing-dependent and therefore extremely hard to reproduce.

**The fix**: identify and **protect** the region of code that touches shared data — the **critical section** — so that only one thread is inside it at a time.

> **Why race conditions are so feared in practice**: They depend on timing, so they may pass every test in development and only manifest in production under load. The bug that escaped Therac-25 (radiation-therapy machine, 1985–87) and killed patients was a race condition in operator-input handling. Synchronization is not a stylistic concern — it is a correctness concern.

### 1.3 Race Conditions in the Kernel

The kernel manipulates extensive shared state on behalf of all processes, so race conditions there can corrupt the entire system.

- **`fork()`** maintains a `next_available_pid` counter. Two simultaneous `fork()` calls can read the same value before either updates it → **two processes share one PID**.
- **Open file table** modifications can collide when multiple processes open files concurrently.
- **Memory allocator** bookkeeping (free lists, zone counts) can be corrupted.
- **Process list** updates during `fork()`/`exit()` and **interrupt handlers** that touch shared structures are similarly vulnerable.

Kernel developers must **systematically prevent** these races; user-space programmers benefit from the kernel's discipline.

### 1.4 The Critical-Section Problem

Each process is structured around a critical section as follows:

```c
while (true) {
    /* entry section */
        /* critical section */
    /* exit section */
        /* remainder section */
}
```

A correct solution to the Critical-Section Problem must satisfy **three requirements**:

1. **Mutual Exclusion** — at most one process is in its critical section at any time.
2. **Progress** — if no process is in the critical section and some processes wish to enter, the choice of which one enters cannot be postponed indefinitely. (Processes in the remainder section do not block anyone.)
3. **Bounded Waiting** — once a process requests entry, there is an upper bound on the number of times other processes are allowed to enter the critical section before the request is granted.

> **What each requirement protects against**:
> - Without **Mutual Exclusion** → race conditions return.
> - Without **Progress** → the system can deadlock-by-design (everyone waits even though the CS is empty).
> - Without **Bounded Waiting** → starvation — a particular process may never enter, even though others keep entering.

### 1.5 Preemptive vs Nonpreemptive Kernels

A kernel must choose whether one of its own threads can be preempted while running in kernel mode.

- **Nonpreemptive kernel** — kernel-mode code runs until it blocks, exits, or yields. Race conditions on kernel data are **inherently avoided** on a single CPU (no other kernel thread can interleave with you). Drawback: **poor responsiveness** (high-priority user tasks may have to wait while the kernel finishes).
- **Preemptive kernel** — kernel-mode code itself can be preempted. Better responsiveness and more suitable for real-time work, but the kernel must be carefully designed to handle race conditions, especially in **SMP** systems.

Two practical points:
- **Single processor** systems can implement mutual exclusion cheaply by **disabling interrupts** for the critical section.
- **Multiprocessor** systems cannot — disabling interrupts on every core is expensive — so they need other mechanisms (memory barriers, atomic instructions, locks).

> **Historical note**: Linux was nonpreemptive until kernel 2.6 (2003), then became fully preemptive. Modern Linux, Windows, and macOS are all preemptive kernels designed to scale across many cores.

---

<br>

## 2. Peterson's Solution

### 2.1 Algorithm

Peterson's Solution is a classic **software-only** solution to the Critical-Section Problem for **two** processes (P0 and P1). It uses two shared variables:

```c
int     turn;       /* whose turn it is to enter */
boolean flag[2];    /* flag[i] = true means process i wants to enter */
```

Each process Pi (with its partner Pj, where `j = 1 - i`) executes:

```c
while (true) {
    flag[i] = true;        /* announce: I want in */
    turn = j;              /* yield: it's your turn */
    while (flag[j] && turn == j)
        ;                  /* wait while the other wants in AND it's their turn */

    /* critical section */

    flag[i] = false;       /* I am done */

    /* remainder section */
}
```

Intuition: **`flag[i]` says "I want in"**, and **`turn` says "you go first"**. A process enters the critical section only if its partner does not want in (`flag[j] == false`) **or** its partner has already yielded (`turn == i`).

### 2.2 Correctness Proof

**1. Mutual Exclusion.** For Pi to leave the entry-section loop, either `flag[j] == false` or `turn == i`. If both processes are in the CS simultaneously, then `flag[0] == flag[1] == true`, so the only way out for both was via `turn`. But `turn` is a single integer that holds either 0 or 1 — it cannot equal both `i = 0` and `i = 1` at once. Contradiction → mutual exclusion holds.

**2. Progress.** If Pj is in the remainder section, `flag[j] == false`, so Pi enters immediately. If Pj has set `flag[j] = true` and is in its entry section, then `turn` decides — and since `turn` is set to a single value, exactly one of the two enters.

**3. Bounded Waiting.** Suppose Pi is waiting. When Pj exits the CS, it sets `flag[j] = false`, and Pi enters. If Pj wants to re-enter, it must first set `turn = i`, so Pi will enter before Pj re-enters. Pj can therefore "cut in line" **at most once** before Pi runs → **bound = 1**.

> **Why Peterson's is famous despite being obsolete in practice**: It proves that mutual exclusion is achievable with **only ordinary loads and stores** on shared memory — no special hardware instructions required, in theory. That theoretical clarity is why it appears in every OS textbook.

### 2.3 Limitations on Modern Architectures

Peterson's correctness assumes that statements execute **in program order** and that a write becomes visible to other CPUs immediately. **Modern processors and compilers violate both assumptions** for performance — they routinely **reorder** independent loads and stores, and a store may sit in a per-core write buffer for some time before becoming globally visible.

```text
/* Peterson's entry section */
flag[i] = true;     /* ① */
turn = j;           /* ② */
```

If a CPU executes ② before ①, both processes may pass the entry check and enter the CS simultaneously — **mutual exclusion is broken**. The same effect can arise from a compiler reordering (since the two writes target unrelated memory).

*Silberschatz Figure 6.4 — The effects of instruction reordering in Peterson's solution.*

**Fix**: insert **memory barriers** (next section) that prevent the hardware and compiler from reordering across the boundary. With proper barriers, Peterson's algorithm does work correctly on modern hardware — but in practice, programmers use higher-level primitives (locks, atomics) that hide this complexity.

> **[Computer Architecture]** An algorithm that is provably correct on an idealized **sequentially-consistent** memory model can fail catastrophically on real hardware that implements a **relaxed (weak) memory model**. A machine's memory model — which load/store reorderings the CPU and compiler are *allowed* to perform — is part of the architecture specification: x86 is relatively strong (TSO — Total Store Order); ARM and POWER are much weaker. Synchronization code must explicitly respect this with **memory barriers** (next section) or with higher-level primitives that insert barriers internally.

---

<br>

## 3. Hardware Support for Synchronization

### 3.1 Memory Barriers (Memory Fences)

The **memory model** of a system describes the guarantees the hardware gives about the visibility and ordering of memory operations across CPUs:

- **Strongly ordered** — a write by one processor is **immediately visible** to all others, in program order. (Easy to reason about, but slower.)
- **Weakly ordered** — writes can become visible **out of order** and after a delay. (Faster — the dominant choice in modern multicore designs, e.g., ARM, x86 with caveats.)

A **Memory Barrier** (or **memory fence**) is a special instruction that **enforces ordering**: all loads and stores **before** the barrier must complete before any load or store **after** the barrier may begin (from the viewpoint of other processors).

```c
/* Thread 1 */                       /* Thread 2 */
while (!flag)                        x = 100;
    ;                                memory_barrier();
memory_barrier();                    flag = true;
print x;  /* guaranteed to print 100 */
```

Without the barriers, Thread 2's write to `x` could become visible **after** the write to `flag`, so Thread 1 might see `flag == true` but `x == 0`. The barriers force the program order to hold globally.

- Memory barriers are a **very low-level** primitive used mostly by **kernel and library writers**. Most application programmers use locks or atomics, which insert the necessary barriers internally.
- With appropriate barriers, **Peterson's Solution works correctly on modern hardware**.

> **[Computer Architecture]** Mental model for memory barriers: think of each CPU core as having a **store buffer** between its pipeline and the cache-coherent memory system. A `store` instruction *retires* into this buffer immediately so the pipeline can keep moving; the buffer drains to memory later in some order chosen by the hardware. That delayed drain is what lets one core's writes appear out-of-order to another core. A memory barrier (`mfence`/`sfence` on x86, `dmb`/`dsb` on ARM) is the instruction that says "drain my store buffer before I do anything else, and don't let later operations overtake earlier ones."

### 3.2 test_and_set

`test_and_set` is an **atomic** instruction that returns the old value of a memory cell and unconditionally writes `true` to it — both as a single, indivisible operation.

```c
boolean test_and_set(boolean *target) {
    boolean rv = *target;
    *target = true;
    return rv;             /* old value */
}
```

**Mutual exclusion via `test_and_set`** (lock initialized to `false`):

```c
do {
    while (test_and_set(&lock))
        ;                  /* busy wait until lock was false */

    /* critical section */

    lock = false;

    /* remainder section */
} while (true);
```

The first thread to call `test_and_set` sees `false`, sets it to `true`, and enters. Every subsequent caller sees `true` and spins. Even if two CPUs call `test_and_set` at exactly the same moment, the hardware **serializes** them — only one will read `false`.

### 3.3 compare_and_swap (CAS)

A more general atomic primitive: **compare-and-swap** atomically tests whether a memory cell holds an expected value, and if so, replaces it with a new one.

```c
int compare_and_swap(int *value, int expected, int new_value) {
    int temp = *value;
    if (*value == expected)
        *value = new_value;
    return temp;           /* always returns the original value */
}
```

**Mutual exclusion using CAS** (lock initialized to 0; 0 = free, 1 = held):

```c
while (true) {
    while (compare_and_swap(&lock, 0, 1) != 0)
        ;                  /* spin until lock was 0 */

    /* critical section */

    lock = 0;

    /* remainder section */
}
```

- On Intel x86, CAS is implemented by the **`lock cmpxchg`** instruction — an `xchg` variant prefixed by `lock` so the bus is held for the duration.
- CAS is the primitive at the bottom of nearly every modern lock-free data structure (lock-free queues, hazard pointers, RCU, …).

> **Why CAS is more powerful than test_and_set**: `test_and_set` works only on booleans; CAS works on arbitrary integers and lets you say "update this if it still holds the value I last saw" — the foundation of optimistic concurrency, where a thread reads a value, computes a new one, and only commits if no one else has changed the value in the meantime.

### 3.4 CAS — Ensuring Bounded Waiting

The plain CAS-spin lock above guarantees mutual exclusion but **not bounded waiting** — an unlucky thread can be repeatedly beaten to the lock. The following extension uses a `waiting[]` flag array to ensure every requester is served within at most `n - 1` other CS entries:

```c
/* Shared: boolean waiting[n] = {false}, int lock = 0 */

while (true) {
    waiting[i] = true;                          /* I am waiting */
    key = 1;
    while (waiting[i] && key == 1)
        key = compare_and_swap(&lock, 0, 1);    /* try to grab lock */
    waiting[i] = false;

    /* critical section */

    j = (i + 1) % n;
    while ((j != i) && !waiting[j])
        j = (j + 1) % n;                        /* find next waiter */

    if (j == i)
        lock = 0;                               /* nobody waiting */
    else
        waiting[j] = false;                     /* hand the lock to next */

    /* remainder section */
}
```

When a thread exits the CS, it scans `waiting[]` **cyclically** starting from its successor and hands the lock to the first waiter found. Every requester is therefore served within at most `n - 1` other entries → **bounded waiting**.

### 3.5 Atomic Variables

**Atomic variables** are a higher-level abstraction built on CAS that gives you race-free updates of single integers without writing the CAS loop yourself.

A canonical CAS-based atomic increment:

```c
void increment(atomic_int *v) {
    int temp;
    do {
        temp = *v;
    } while (temp != compare_and_swap(v, temp, temp + 1));
}
```

If the value was changed by another thread in the meantime, the CAS fails and the loop retries with the fresh value.

**Linux kernel API**:

```c
atomic_t counter;
atomic_set(&counter, 5);                /* counter = 5  */
atomic_add(10, &counter);               /* counter = 15 */
atomic_sub(4, &counter);                /* counter = 11 */
atomic_inc(&counter);                   /* counter = 12 */
int val = atomic_read(&counter);        /* val     = 12 */
```

**Important caveat**: atomic variables protect updates to **a single variable**. Compound invariants involving multiple variables — e.g., the bounded-buffer invariants `0 ≤ count ≤ n` and `in/out` indices — still need a **mutex** or **semaphore** that protects the whole critical section.

> **A useful rule of thumb**: If all you need to do is "increment a counter" or "set a flag", use an atomic. If you need to update **two related fields together** so that no observer ever sees them out of sync, use a lock.

---

<br>

## 4. Mutex Locks

### 4.1 Mutex Lock — Basics

A **mutex lock** ("**mut**ual **ex**clusion") is the simplest synchronization tool: a Boolean `available` flag with two operations.

```c
acquire() {
    while (!available)
        ;                  /* busy wait */
    available = false;
}

release() {
    available = true;
}
```

Usage:

```c
acquire(lock);
    /* critical section */
release(lock);
```

Two requirements that the textbook formulation hides:

- `acquire()` and `release()` themselves must be **atomic**. They are typically implemented on top of `test_and_set` or CAS.
- The lock variable is a **Boolean** — exactly two states (held / free).

A mutex's job is to make the critical section appear **as if it executed in isolation**.

### 4.2 Spinlocks and Busy Waiting

A **spinlock** is a mutex whose `acquire()` **busy-waits** (loops) until the lock is free, instead of putting the thread to sleep.

| Property | Spinlock | Sleeping lock |
|----------|----------|---------------|
| Wait strategy | **Spin** in a loop | **Block** (sleep) |
| Wakeup cost | None — already running | Two context switches (sleep + wake) |
| Best when | Lock held for a **very short** time | Lock held for a **longer** time |
| Risk | Wastes CPU on uniprocessor; only one core can spin productively | Higher latency to acquire |

**Spinlock advantages**:
- **No context-switch overhead** — when the lock is released, the spinning thread sees it on the next loop iteration and continues.
- **Especially useful in multicore systems**: while the lock is held by core A, core B can spin productively, and the moment A releases, B grabs it without involving the scheduler.
- **Rule of thumb**: use a spinlock when the expected hold time is **shorter than two context switches**.

**Spinlock disadvantages**:
- Wastes CPU cycles. On a **uniprocessor** with no other useful work to do, spinning is purely wasteful (the holder cannot release while the spinner is using its CPU slot).
- High **lock contention** (many threads contending for the same lock) destroys performance.

**Lock contention terminology**:
- **Uncontended** lock — one thread tries to acquire and succeeds immediately.
- **Contended** (blocked) lock — multiple threads are simultaneously trying to acquire; some must wait.

> **The deeper picture**: Spinlocks vs sleeping locks is a *time-cost* trade-off. Sleeping pays a fixed two-context-switch cost; spinning pays a cost proportional to wait time. Whichever is smaller wins — which is why kernels often use spinlocks for very-short kernel-data critical sections, and sleeping locks (mutexes) for longer user-level work.

---

<br>

## 5. Semaphores

### 5.1 Definition (wait / signal)

A **semaphore** S is an integer variable accessed only through two **atomic** operations, originally proposed by Edsger Dijkstra:

**`wait(S)`** — also called **P** (Dutch *proberen*, "to test"):

```c
wait(S) {
    while (S <= 0)
        ;          /* busy wait */
    S--;
}
```

**`signal(S)`** — also called **V** (Dutch *verhogen*, "to increment"):

```c
signal(S) {
    S++;
}
```

Two atomicity requirements that are easy to miss:
- The modifications to `S` must be atomic (`S--` and `S++` are decomposed read-modify-writes).
- In `wait`, **the test of `S` and the decrement together must be uninterruptible** — otherwise two threads can both observe `S = 1` and both decrement, ending up with `S = -1` when only one should have entered.

### 5.2 Semaphore Types and Usage

**Counting semaphore.**
- Range: any non-negative integer.
- Models a **resource pool of N identical resources**.
- `wait(S)` acquires one resource (S decreases), `signal(S)` releases one (S increases).
- When `S = 0`, all resources are in use → further `wait()` calls block.

**Binary semaphore.**
- Range: 0 or 1.
- Behaves **almost identically to a mutex lock** — initialized to 1, `wait` = lock, `signal` = unlock.

**Execution-order control with a semaphore.** A semaphore can also enforce that one statement must execute before another. With `synch` initialized to 0:

```text
P1:                           P2:
    S1;                           wait(synch);
    signal(synch);                S2;
```

`P2` blocks at `wait(synch)` until `P1` calls `signal(synch)` after executing `S1`. So **S2 is guaranteed to execute after S1**, regardless of the order in which P1 and P2 are scheduled.

> **The key generalization over mutexes**: a counting semaphore is the natural tool whenever there are **N indistinguishable resources** (database connections, threadpool slots, free buffer slots, …). Mutex = "one access at a time"; counting semaphore = "at most N accesses at a time".

### 5.3 Eliminating Busy Waiting (Waiting Queue)

The textbook definition of `wait()` busy-waits, which is wasteful when the CS is long. Real implementations attach a **waiting queue** of blocked processes to each semaphore:

```c
typedef struct {
    int value;
    struct process *list;     /* waiting queue */
} semaphore;

wait(semaphore *S) {
    S->value--;
    if (S->value < 0) {
        /* enqueue this process on S->list */
        sleep();              /* block until woken */
    }
}

signal(semaphore *S) {
    S->value++;
    if (S->value <= 0) {
        /* dequeue some process P from S->list */
        wakeup(P);
    }
}
```

Two consequences worth memorizing:

- When `S->value` is **negative**, its **absolute value equals the number of processes blocked on this semaphore**.
- A **FIFO** waiting queue gives **bounded waiting** (every blocked process is eventually woken in order).

Note that `wait` now decrements first and then checks — this matches the negative-value bookkeeping above.

### 5.4 Atomicity of Semaphore Implementation

For `wait()` and `signal()` to behave correctly, their bodies must execute **atomically**.

- **Single processor**: just **disable interrupts** during the wait/signal body. Cheap and effective.
- **Multiprocessor**: disabling interrupts on every CPU is expensive and bad for latency. Implementations use a **CAS** or a **spinlock** internally to make the wait/signal body atomic across cores.

Note that this **does not eliminate busy waiting completely** — it merely shrinks it from the application's potentially long CS down to the few instructions inside `wait`/`signal`. The application still benefits from sleep/wake-up because *its* CS no longer requires busy waiting.

> **The picture to keep in mind**: synchronization primitives form a **stack**. At the bottom is hardware (CAS, fences); on top of CAS sits a tiny internal spinlock; on top of that sits the semaphore (which exposes a sleep/wake interface). Each layer hides the busy-waiting of the layer below.

---

<br>

## 6. Monitors

### 6.1 Difficulty of Using Semaphores

Semaphores are powerful but **error-prone**. A misuse causes a timing bug that may only manifest under specific interleavings, making it brutal to debug. Common mistakes:

- **Order reversal** — `signal(mutex); CS; wait(mutex);` → multiple processes can enter the CS at once → **mutual exclusion violation**.
- **Duplicate wait** — `wait(mutex); CS; wait(mutex);` → the second `wait` blocks forever → **deadlock**.
- **Omitted wait or signal** — leaves the CS unprotected, or never wakes anyone up.

These bugs share three properties: (1) the program looks reasonable, (2) tests usually pass, (3) failure is rare and timing-dependent. The remedy is a **higher-level construct** that hides the explicit `wait`/`signal` calls — the **monitor**.

### 6.2 Monitor — Structure

A **monitor** is an **abstract data type (ADT)** that automatically provides mutual exclusion among its operations: at any moment, at most one thread is executing inside the monitor.

```text
monitor monitor_name {
    /* shared variable declarations (private) */

    function P1(...) { ... }
    function P2(...) { ... }
    ...
    function Pn(...) { ... }

    initialization_code(...) { ... }
}
```

*Silberschatz Figure 6.12 — Schematic view of a monitor.*

Key properties:
- **Only one thread at a time** can be active inside the monitor — the compiler/runtime, not the programmer, inserts the lock/unlock around each function.
- The internal variables are accessible **only via the monitor's functions** — no outsider can corrupt them.
- The programmer never writes `wait(mutex)` / `signal(mutex)` for the monitor lock.

> **Why this is a leap forward**: with semaphores, mutual exclusion is a programmer **convention** ("everyone who touches X must call wait first"). With monitors, mutual exclusion is a **structural** property of the ADT — there is no way to access X without going through a monitor function, and the monitor function is automatically protected.

### 6.3 Condition Variables

Mutual exclusion alone is not enough — sometimes a thread inside the monitor must **wait** for a condition that another monitor function can establish. **Condition variables** provide that:

```text
condition x, y;

x.wait();      /* the calling thread is suspended on x */
x.signal();    /* wake up one thread waiting on x */
```

Two essential differences from semaphores:

- `x.signal()` has **no effect** if no thread is waiting on `x`. In a semaphore, `signal` always changes the value.
- A thread that calls `x.wait()` **releases the monitor** so other threads can enter; otherwise the monitor would deadlock.

**What happens after `x.signal()` when both the signaler P and the awakened Q want to be inside the monitor?** Three policies exist:

- **Signal and wait** — P waits, Q runs immediately. (Original Hoare definition.)
- **Signal and continue** — P continues until it leaves the monitor; Q runs afterward.
- **Compromise** — P leaves the monitor immediately after signaling; Q resumes.

*Silberschatz Figure 6.13 — Monitor with condition variables.*

> **What condition variables are *for***: they let a thread **politely give up the monitor** while waiting for a state change (e.g., "buffer is non-empty"), and let another thread **wake** that waiter when the state changes. Without condition variables, a waiting thread would either spin (wasting CPU) or block while still holding the monitor lock (deadlock).

### 6.4 Implementing a Monitor with Semaphores

Monitors can be built on semaphores. Here is the **signal-and-wait** version. Each external monitor function `F` is wrapped:

```text
semaphore mutex      = 1;     /* protects monitor entry */
semaphore next       = 0;     /* signaler waits here */
int       next_count = 0;     /* # signalers blocked on next */

/* Body of every external function F */
wait(mutex);
    ... body of F ...
if (next_count > 0)
    signal(next);             /* wake a waiting signaler */
else
    signal(mutex);            /* otherwise release the monitor lock */
```

Each condition variable `x` needs its own semaphore `x_sem` and counter `x_count`:

```text
/* x.wait() */                          /* x.signal() */
x_count++;                              if (x_count > 0) {
if (next_count > 0)                         next_count++;
    signal(next);                            signal(x_sem);
else                                         wait(next);
    signal(mutex);                           next_count--;
wait(x_sem);                            }
x_count--;
```

The bookkeeping is intricate — which is precisely the *point*: the monitor abstraction hides this from the programmer, and the compiler/runtime is the one party that must get it right.

### 6.5 ResourceAllocator Monitor Example

A monitor that allocates a single resource, prioritizing whoever requests it for the **shortest** time (using a `condition.wait(time)` priority form):

```text
monitor ResourceAllocator {
    boolean   busy;
    condition x;

    void acquire(int time) {
        if (busy)
            x.wait(time);     /* time = priority; smaller = sooner */
        busy = true;
    }

    void release() {
        busy = false;
        x.signal();           /* wake the waiter with smallest time */
    }

    initialization_code() {
        busy = false;
    }
}
```

Usage:

```text
R.acquire(t);
    ... use the resource ...
R.release();
```

`x.wait(c)` stores priority value `c` for the caller; `x.signal()` resumes the waiter with the **smallest** stored value. This is a **conditional-wait** variant supported by some monitor implementations.

---

<br>

## 7. Liveness

A **liveness** failure is one where individual processes are alive but the system never makes progress on what they want to do. Mutual exclusion is about **safety**; liveness is about **eventually getting your turn**.

### 7.1 Deadlock

**Deadlock** is a state where every process in a set is waiting for an event that **only another process in the same set can cause**. None can ever proceed.

Classic two-process deadlock with two semaphores S, Q (both initialized to 1):

```text
P0                          P1
wait(S);                    wait(Q);
wait(Q);                    wait(S);
...                         ...
signal(S);                  signal(Q);
signal(Q);                  signal(S);
```

If P0 acquires S and is then preempted right before `wait(Q)`, while P1 acquires Q and is preempted right before `wait(S)` — **each holds what the other needs**, neither can proceed.

- Deadlock most often arises around **resource acquisition / release**.
- Diagnosis, prevention, avoidance, detection, and recovery for deadlock are covered in detail in **Ch 8 (next week)**.

> **The four necessary conditions** (preview of Ch 8): mutual exclusion, hold and wait, no preemption, circular wait. Eliminating any one is enough to prevent deadlock — a useful framing once you know what to look for.

### 7.2 Priority Inversion and Priority Inheritance

**Priority inversion** is a subtle liveness bug where a **lower-priority** process indirectly **blocks a higher-priority** one.

The setup — three priorities **L** (low) < **M** (medium) < **H** (high):

1. **L** acquires semaphore S.
2. **H** becomes runnable and tries to acquire S → blocked, waiting for L.
3. **M** becomes runnable. M has higher priority than L, so M **preempts L**.
4. As long as M runs, L cannot release S, and H cannot proceed.

Net result: **H is delayed by M**, even though H has higher priority than M.

**Priority Inheritance Protocol**:
- While L holds a lock that H is waiting for, **temporarily raise L to H's priority**.
- M can no longer preempt L → L finishes its CS quickly and releases S.
- After L releases the lock, it reverts to its original priority.

> **A famous real incident — Mars Pathfinder, 1997.** The lander suffered repeated, mysterious system resets on Mars. The cause was a priority-inversion bug in **VxWorks RTOS**: a low-priority meteorological task held a mutex that a high-priority bus-management task needed, and a medium-priority comms task kept preempting the low-priority one. A watchdog timer expired and reset the system. JPL engineers fixed it remotely by **enabling priority inheritance** on the offending mutex. Synchronization is real engineering, with real consequences.

### 7.3 Evaluating Synchronization Tools

Synchronization tools differ in **performance** depending on **lock contention**:

| Contention level | CAS-based (optimistic) | Locking (pessimistic) |
|------------------|------------------------|-----------------------|
| **Uncontended** | Fast (slight edge) | Fast |
| **Moderate** | **Much faster** — no context switch | Slower — pays context-switch cost |
| **High** | Slow — many CAS retries | Fast — queue-based ordering |

A useful tier-of-abstraction view:

| Level | Tools | Typical user |
|-------|-------|--------------|
| **Low** | Hardware (CAS, test_and_set, fences) | Mostly used as the *foundation* of higher tools |
| **Mid** | Mutex locks, semaphores | Most common in kernels and applications |
| **High** | Monitors, condition variables | Language-level support (Java, C# `lock`, …) |

> **Practical implication**: there is no single "best" synchronization primitive. The right choice depends on the **expected contention**, the **length of the critical section**, and whether you need **multi-resource** or **single-flag** semantics.

---

<br>

## 8. Classic Problems of Synchronization

Three classical problems recur throughout systems software. Each illustrates a distinct synchronization pattern.

### 8.1 Bounded-Buffer Problem

A **bounded buffer of size n** is shared between **producers** that insert items and **consumers** that remove them. Three semaphores:

| Semaphore | Init | Role |
|-----------|------|------|
| `mutex` | 1 | Protects the buffer (binary). |
| `empty` | n | Number of empty slots (counting). |
| `full`  | 0 | Number of filled slots (counting). |

Shared declarations:

```c
int n;
semaphore mutex = 1;
semaphore empty = n;
semaphore full  = 0;
```

`empty` and `full` are **counting** semaphores; `mutex` is a **binary** semaphore for buffer access.

### 8.2 Bounded-Buffer — Producer/Consumer Code

```c
/* Producer */                       /* Consumer */
while (true) {                       while (true) {
    /* produce an item */                wait(full);
                                         wait(mutex);
    wait(empty);
    wait(mutex);                         /* remove item from buffer */
                                         /* into next_consumed       */
    /* add item to buffer */
                                         signal(mutex);
    signal(mutex);                       signal(empty);
    signal(full);
}                                        /* consume next_consumed */
                                     }
```

Two non-obvious correctness points:

- **Wait order matters**: always `wait(empty)` (or `wait(full)`) **before** `wait(mutex)`. If a producer reverses the order to `wait(mutex); wait(empty);`, then when the buffer is full it holds `mutex` while blocked on `empty` — and the consumer cannot acquire `mutex` to free a slot. **Deadlock.**
- The producer and consumer are **structurally symmetric** (`empty` ↔ `full`). Whenever you see this pattern, try to verify the symmetry — it is often the easiest correctness check.

> **Why three semaphores instead of one mutex**: a single mutex can ensure mutual exclusion on the buffer, but it cannot make a producer **wait** until the buffer has an empty slot. The two counting semaphores express the *availability* of slots; the mutex serializes the *act* of inserting/removing.

### 8.3 Readers-Writers Problem

A shared dataset is accessed by two kinds of processes:

- **Readers** — read the data only.
- **Writers** — read and modify.

The synchronization rules:

- **Many readers may read concurrently** (they don't conflict).
- **A writer needs exclusive access** — no other writers and no readers.

Two variants of the problem differ in whose preferences win:

- **First Readers-Writers Problem (readers priority).** A reader may enter even while writers are waiting. Writers can **starve**.
- **Second Readers-Writers Problem (writers priority).** Once a writer is waiting, no new reader may enter. Readers can **starve**.

Both versions allow **starvation** — only the victim differs. A **reader-writer lock** generalizes mutexes to distinguish **read mode (shared)** and **write mode (exclusive)**, leaving the priority policy as a parameter.

### 8.4 Readers-Writers — First Problem Solution

Shared variables:

```c
semaphore rw_mutex = 1;     /* writer-vs-readers exclusion */
semaphore mutex    = 1;     /* protects read_count        */
int       read_count = 0;
```

Code:

```c
/* Writer */                            /* Reader */
while (true) {                          while (true) {
    wait(rw_mutex);                         wait(mutex);
                                            read_count++;
    /* writing is performed */              if (read_count == 1)
                                                wait(rw_mutex);
    signal(rw_mutex);                       signal(mutex);
}
                                            /* reading is performed */

                                            wait(mutex);
                                            read_count--;
                                            if (read_count == 0)
                                                signal(rw_mutex);
                                            signal(mutex);
                                        }
```

Reading the protocol:

- The **first reader** acquires `rw_mutex` (locking out writers); subsequent readers don't bother.
- The **last reader** releases `rw_mutex` (letting any waiting writer in).
- `mutex` only protects the `read_count` counter and is held briefly.

When n readers and 1 writer are all waiting: 1 reader holds `rw_mutex`, n − 1 readers wait on `mutex`, and the writer waits on `rw_mutex`.

### 8.5 Reader-Writer Locks

A **reader-writer lock** is the generalization of a mutex that makes the reader/writer distinction first-class.

- **Read mode** — multiple threads may hold the lock simultaneously.
- **Write mode** — only one thread may hold the lock; no readers may.

Reader-writer locks pay off when:
- Read and write operations can be cleanly distinguished in the code.
- **Readers vastly outnumber writers** — the cost of the more complex lock is repaid by the higher concurrency among readers.

API examples:
- POSIX: `pthread_rwlock_t`.
- Java: `java.util.concurrent.locks.ReentrantReadWriteLock`.
- Windows: **Slim Reader-Writer Lock** (SRWL).

### 8.6 Dining-Philosophers Problem

Five philosophers sit around a circular table. Between every pair of adjacent philosophers there is one chopstick — five chopsticks total. Each philosopher alternates between **thinking** and **eating**, but to eat she must **hold both adjacent chopsticks**.

Rules:
- Pick up only one chopstick at a time.
- Both left and right chopsticks must be available before eating begins.
- After eating, put both chopsticks down.

*Silberschatz Figure 7.5 — The dining philosophers.*

The dining-philosophers problem is the canonical illustration of **the difficulty of allocating multiple resources to multiple processes simultaneously without deadlock or starvation**.

### 8.7 Dining-Philosophers — Semaphore Solution and Deadlock

Represent each chopstick as a binary semaphore initialized to 1:

```c
semaphore chopstick[5];   /* each initialized to 1 */

/* Philosopher i */
while (true) {
    wait(chopstick[i]);              /* left  */
    wait(chopstick[(i + 1) % 5]);    /* right */

    /* eat for a while */

    signal(chopstick[i]);
    signal(chopstick[(i + 1) % 5]);

    /* think for a while */
}
```

This solution is **incorrect**. If all five philosophers simultaneously execute `wait(chopstick[i])` (each picks up her left), every chopstick value is 0; each philosopher then blocks on `wait(chopstick[(i+1)%5])`. **Circular wait → deadlock**, and no one ever eats.

### 8.8 Dining-Philosophers — Deadlock-Avoidance Strategies

Three classic remedies:

- **Method 1 — limit to at most 4 simultaneous diners.** Add an extra `seats` semaphore initialized to 4. With at most 4 philosophers competing for 5 chopsticks, at least one can always pick up both.
- **Method 2 — pick up chopsticks only when both are available.** Wrap the check-and-pick-up in a critical section so no philosopher ever holds just one chopstick. This is the approach the monitor solution implements.
- **Method 3 — asymmetric protocol.** Odd-numbered philosophers grab **left first**, even-numbered grab **right first**. This breaks the **circular wait** condition (the four conditions for deadlock are covered next week in Ch 8).

Note: **deadlock-free does not imply starvation-free.** A philosopher whose neighbors are both extremely hungry could in principle never eat. Eliminating starvation requires additional fairness mechanisms.

### 8.9 Dining-Philosophers — Monitor Solution

The monitor solution makes "pick up both chopsticks when both are free" a structurally enforced invariant.

```text
monitor DiningPhilosophers {
    enum {THINKING, HUNGRY, EATING} state[5];
    condition self[5];

    void pickup(int i) {
        state[i] = HUNGRY;
        test(i);
        if (state[i] != EATING)
            self[i].wait();
    }

    void putdown(int i) {
        state[i] = THINKING;
        test((i + 4) % 5);    /* re-check left neighbor  */
        test((i + 1) % 5);    /* re-check right neighbor */
    }

    void test(int i) {
        if (state[(i + 4) % 5] != EATING &&
            state[i] == HUNGRY &&
            state[(i + 1) % 5] != EATING) {
            state[i] = EATING;
            self[i].signal();
        }
    }

    initialization_code() {
        for (int i = 0; i < 5; i++)
            state[i] = THINKING;
    }
}
```

Each philosopher i invokes the monitor as:

```text
DiningPhilosophers.pickup(i);
    ...
    /* eat */
    ...
DiningPhilosophers.putdown(i);
```

How it works:

- `pickup(i)` sets state to HUNGRY, calls `test(i)`. If both neighbors are not EATING, state becomes EATING and the philosopher proceeds. Otherwise `self[i].wait()` blocks her.
- `putdown(i)` sets state to THINKING and re-tests both neighbors — if either is hungry and her neighbors aren't eating, that neighbor's wait is signaled.
- **Deadlock-free**: a philosopher only eats when **both** chopsticks are available, so no one holds half a pair.
- **Starvation possible**: an unlucky philosopher whose neighbors keep alternating eating could be perpetually skipped (left as an exercise in the textbook).

---

<br>

## 9. Synchronization in Real Systems

### 9.1 Windows Synchronization

Windows distinguishes single-processor and multiprocessor cases, and offers both kernel-side and user-side primitives.

- **Single processor** — protect global state by **masking interrupts** in the kernel.
- **Multiprocessor** — use **spinlocks** for very short kernel critical sections. A thread is **never preempted while holding a spinlock** (otherwise the holder might sleep while a spinner burns CPU forever).

**Dispatcher Objects** — kernel objects exposed to user threads for synchronization.
- Examples: **Mutex Lock**, **Semaphore**, **Event**, **Timer**.
- Each has two states: **signaled** (available) and **nonsignaled** (in use).
- A thread that requests a nonsignaled object is **blocked** and placed on the object's waiting queue.
- When the object becomes signaled, a waiter is moved to the **ready** state.

**Critical-Section Object** — a user-mode mutex optimized for the common case.
- Tries to acquire with a **spinlock first** (avoiding any kernel call).
- If the spin takes too long, allocates a **kernel mutex** and blocks.
- In the **uncontended** case, acquire/release happens entirely in user mode → very fast.

### 9.2 Linux Synchronization

Linux has been **fully preemptive** since kernel **2.6** (previously nonpreemptive).

| Tool | Purpose | Notes |
|------|---------|-------|
| `atomic_t`     | Atomic integer ops | Counters, sequence generators |
| `spinlock_t`   | Short kernel CS (SMP) | On a uniprocessor, replaced by **disable preemption** |
| `mutex`        | Longer CS | **Sleeping** lock — blocks rather than spins |
| `semaphore`    | Counting semaphore | Suitable for long waits |

Single-CPU vs SMP behavior of the spinlock primitive:

```c
/* Single Processor          | Multiple Processors    */
/* Disable kernel preempt    | Acquire spin lock      */
/* ... critical section ...                           */
/* Enable kernel preempt     | Release spin lock      */
```

Two important properties of Linux spinlocks and mutexes:

- **Nonrecursive** — a thread that already holds a lock will **deadlock** if it tries to acquire it again.
- The kernel maintains a per-task `preempt_count` — preemption is allowed only when `preempt_count == 0`. Holding any lock increments this counter.

### 9.3 POSIX Mutex Locks

POSIX (UNIX, Linux, macOS) provides synchronization primitives at the **user level** through `pthread`.

```c
#include <pthread.h>

pthread_mutex_t mutex;
pthread_mutex_init(&mutex, NULL);

pthread_mutex_lock(&mutex);
    /* critical section */
pthread_mutex_unlock(&mutex);
```

This is the standard mutex idiom you will see in any pthread program.

### 9.4 POSIX Semaphores (Named and Unnamed)

POSIX provides two flavors of semaphores for different sharing scopes.

**Named semaphores** — identified by a string name, can be shared between **unrelated processes**:

```c
#include <semaphore.h>

sem_t *sem;
sem = sem_open("SEM", O_CREAT, 0666, 1);

sem_wait(sem);
    /* critical section */
sem_post(sem);
```

**Unnamed semaphores** — initialized with `sem_init(&sem, 0, 1)`; shared among **threads of the same process** (or among related processes via shared memory).

| Type | API | Sharing scope |
|------|-----|---------------|
| Named   | `sem_open` / `sem_wait` / `sem_post` | Across **unrelated processes** |
| Unnamed | `sem_init` / `sem_wait` / `sem_post` | Within a **single process** (or via shared memory) |

### 9.5 POSIX Condition Variables

Because C has no built-in monitor, condition variables in POSIX are explicitly paired with a mutex.

Setup:

```c
pthread_mutex_t  mutex;
pthread_cond_t   cond_var;
pthread_mutex_init(&mutex, NULL);
pthread_cond_init(&cond_var, NULL);
```

Waiting thread:

```c
pthread_mutex_lock(&mutex);
while (a != b)
    pthread_cond_wait(&cond_var, &mutex);   /* atomically: release mutex + sleep */
pthread_mutex_unlock(&mutex);
```

Signaling thread:

```c
pthread_mutex_lock(&mutex);
a = b;
pthread_cond_signal(&cond_var);             /* wake one waiter */
pthread_mutex_unlock(&mutex);
```

Three rules every pthread programmer must know:

- `pthread_cond_wait()` **atomically releases the mutex and blocks**; on wakeup, it **reacquires** the mutex before returning.
- `pthread_cond_signal()` does **not** release the mutex — the signaler must call `unlock()` itself.
- The condition test must always be inside a **`while` loop**, not an `if`. Spurious wakeups are permitted by the standard, and other threads may have changed the state between the wakeup and reacquiring the mutex.

> **The "always while, never if" rule**: writing `if (a != b) pthread_cond_wait(...)` is a classic bug. Even if no spurious wakeup ever happens, another waiter may grab the resource you were notified about before you re-check, and you would proceed on a false premise. The `while` re-checks the predicate every time you wake.

---

<br>

## 10. Java Synchronization

Java provides monitor-like synchronization at the **language level**, with the `synchronized` keyword as the primary tool and richer primitives in `java.util.concurrent`.

### 10.1 Java Monitors — `synchronized`, `wait()`/`notify()`

A bounded buffer using Java's built-in monitor:

```java
public class BoundedBuffer<E> {
    private static final int BUFFER_SIZE = 5;
    private int count, in, out;
    private E[] buffer;

    public synchronized void insert(E item) {
        while (count == BUFFER_SIZE) {
            try { wait(); }
            catch (InterruptedException ie) { }
        }
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        notify();
    }

    public synchronized E remove() {
        while (count == 0) {
            try { wait(); }
            catch (InterruptedException ie) { }
        }
        E item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        notify();
        return item;
    }
}
```

The `synchronized` keyword **automatically** acquires the receiver object's monitor lock on entry and releases it on exit (even on exception).

### 10.2 How Java Monitors Work Internally

- Every Java object owns **one lock**, an **entry set** (threads waiting for the lock), and a **wait set** (threads blocked on `wait()`).

**When a thread calls `wait()`**:
1. Releases the object's lock.
2. Transitions to the **blocked** state.
3. Joins the **wait set**.

**When a thread calls `notify()`**:
1. Picks an **arbitrary** thread T from the wait set.
2. Moves T from the wait set into the **entry set**.
3. Marks T runnable.
4. When T eventually re-acquires the lock, its `wait()` call returns.

`notifyAll()` does the same as `notify()` but for **every** thread in the wait set. Use `notifyAll()` when more than one type of waiting condition shares the same lock — only the threads whose condition has actually been satisfied will proceed past their `while` loop.

### 10.3 ReentrantLock and Semaphore

`java.util.concurrent.locks.ReentrantLock` is a more flexible lock than `synchronized`:

```java
Lock key = new ReentrantLock();
key.lock();
try {
    /* critical section */
} finally {
    key.unlock();    /* MUST release in finally */
}
```

Notable features:
- A **fairness parameter** (`new ReentrantLock(true)`) makes the lock prefer the longest-waiting thread.
- **Reentrant**: a thread that already holds the lock may acquire it again without deadlocking (the lock counts depth).

`java.util.concurrent.Semaphore` is the standard semaphore:

```java
Semaphore sem = new Semaphore(1);
try {
    sem.acquire();
    /* critical section */
} catch (InterruptedException ie) { }
finally {
    sem.release();
}
```

### 10.4 Java Condition Variables

`Condition` objects (associated with a `Lock`) let you wait on **named** conditions, so you can wake up a *specific* waiter rather than picking randomly with `notify()`.

```java
Lock lock = new ReentrantLock();
Condition[] condVars = new Condition[5];
for (int i = 0; i < 5; i++)
    condVars[i] = lock.newCondition();

public void doWork(int threadNumber) {
    lock.lock();
    try {
        if (threadNumber != turn)
            condVars[threadNumber].await();

        /* do some work */

        turn = (turn + 1) % 5;
        condVars[turn].signal();
    } catch (InterruptedException ie) { }
    finally {
        lock.unlock();
    }
}
```

- `await()` releases the lock and suspends (analog of `pthread_cond_wait()` and Java `Object.wait()`).
- `signal()` wakes the **specific** waiter on that named condition — the principal advantage over the single anonymous `notify()` of built-in monitors.

> **When named condition variables matter**: in a bounded buffer, "buffer not empty" and "buffer not full" are two distinct conditions. With a single anonymous `notify()`, a producer who fills the last slot might wake another *producer* waiting for an empty slot, who immediately blocks again — a wasted wakeup. With named conditions, the producer signals the consumer-condition specifically, eliminating the spurious round-trip.

---

<br>

## 11. Lab — Dining-Philosophers Implementation

The lab exercises the chapter's concepts on the dining-philosophers problem using POSIX threads.

### 11.1 Lab: Naive pthread Implementation

Each chopstick is a `pthread_mutex_t`. Each philosopher is a thread that loops "think → pick up left → pick up right → eat → put down".

```c
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define N 5

pthread_mutex_t chopstick[N];

void *philosopher(void *arg) {
    int id = *(int *)arg;
    while (1) {
        printf("Philosopher %d is thinking\n", id);
        usleep(rand() % 1000000);

        pthread_mutex_lock(&chopstick[id]);
        pthread_mutex_lock(&chopstick[(id + 1) % N]);

        printf("Philosopher %d is eating\n", id);
        usleep(rand() % 1000000);

        pthread_mutex_unlock(&chopstick[id]);
        pthread_mutex_unlock(&chopstick[(id + 1) % N]);
    }
    return NULL;
}
```

Lab tasks:
- Run the program and observe how it can **deadlock** (every philosopher holding her left chopstick).
- Implement one of the classic remedies (asymmetric protocol, at-most-4 diners, or monitor) and confirm the deadlock disappears.

### 11.2 Lab: Asymmetric Solution

Even-numbered philosophers grab the **right** chopstick first, odd-numbered grab the **left** first — this breaks the circular-wait condition.

```c
void *philosopher_asymmetric(void *arg) {
    int id = *(int *)arg;
    int first, second;

    if (id % 2 == 0) {
        first  = (id + 1) % N;     /* right first */
        second = id;
    } else {
        first  = id;               /* left first */
        second = (id + 1) % N;
    }

    while (1) {
        printf("Philosopher %d is thinking\n", id);
        usleep(rand() % 1000000);

        pthread_mutex_lock(&chopstick[first]);
        pthread_mutex_lock(&chopstick[second]);

        printf("Philosopher %d is eating\n", id);
        usleep(rand() % 1000000);

        pthread_mutex_unlock(&chopstick[first]);
        pthread_mutex_unlock(&chopstick[second]);
    }
    return NULL;
}
```

> **Why it works**: deadlock requires a *cycle* of "P0 waits for P1's chopstick, P1 waits for P2's, …, Pn waits for P0's". In the asymmetric protocol, the order in which any two adjacent philosophers acquire their shared chopstick is reversed, so the cycle cannot close.

### 11.3 Lab: Monitor Solution with POSIX Condition Variables

A direct translation of the textbook monitor (§ 8.9) into POSIX:

```c
#include <pthread.h>
#define N 5

enum {THINKING, HUNGRY, EATING} state[N];
pthread_mutex_t monitor_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  self[N];

void test(int i) {
    if (state[(i + 4) % N] != EATING &&
        state[i] == HUNGRY &&
        state[(i + 1) % N] != EATING) {
        state[i] = EATING;
        pthread_cond_signal(&self[i]);
    }
}

void pickup(int i) {
    pthread_mutex_lock(&monitor_lock);
    state[i] = HUNGRY;
    test(i);
    while (state[i] != EATING)
        pthread_cond_wait(&self[i], &monitor_lock);
    pthread_mutex_unlock(&monitor_lock);
}

void putdown(int i) {
    pthread_mutex_lock(&monitor_lock);
    state[i] = THINKING;
    test((i + 4) % N);
    test((i + 1) % N);
    pthread_mutex_unlock(&monitor_lock);
}
```

Note the use of `while (state[i] != EATING)` around `pthread_cond_wait` — exactly the pattern from § 9.5 (defensive against spurious wakeups).

---

<br>

## Summary

| Concept | Key Summary |
|:--------|:-----------|
| Race Condition | Result depends on interleaving order; arises whenever shared data is touched concurrently without coordination |
| Critical-Section Problem | Requires **Mutual Exclusion**, **Progress**, **Bounded Waiting** |
| Preemptive vs Nonpreemptive Kernel | Nonpreemptive: races avoided on uniprocessor but worse responsiveness; modern kernels (Linux ≥ 2.6, Windows, macOS) are preemptive |
| Peterson's Solution | Software-only, two-process; provably correct but **breaks under instruction reordering** without memory barriers |
| Memory Barrier | Forces ordering of loads/stores across CPUs; foundation under modern locks/atomics |
| `test_and_set` / `compare_and_swap` | Hardware atomic primitives; CAS is the building block of nearly every modern lock-free structure |
| Atomic Variables | Race-free single-variable updates; **insufficient** for multi-variable invariants |
| Mutex Lock | Simplest protection; `acquire()`/`release()` must be atomic |
| Spinlock vs Sleeping Lock | Spin: no context switch, wastes CPU; sleep: scales with long CS — pick by expected hold time |
| Semaphore | Atomic `wait`/`signal`; **counting** for resource pools, **binary** ≈ mutex; can also enforce execution order |
| Waiting Queue | Eliminates application-level busy waiting; `value < 0` ⇒ `|value|` waiters |
| Monitor | High-level ADT with automatic mutual exclusion; replaces error-prone explicit `wait`/`signal` |
| Condition Variable | `wait`/`signal` inside a monitor; signal-and-wait vs signal-and-continue policies |
| Deadlock | Cyclic wait among processes; resolved/prevented in Ch 8 |
| Priority Inversion | Lower-priority task indirectly blocks higher one; fixed by **priority inheritance** (Mars Pathfinder, 1997) |
| Bounded-Buffer | `mutex`, `empty`, `full` semaphores; **wait counting before mutex** to avoid deadlock |
| Readers-Writers | Many readers OR one writer; first/second variants → reader/writer starvation; reader-writer locks |
| Dining-Philosophers | 5 chopsticks, 5 philosophers; naive solution deadlocks; remedies: limit-to-4, asymmetric, monitor |
| Windows | Spinlocks (kernel) + Dispatcher Objects (signaled/nonsignaled) + user-mode Critical-Section objects |
| Linux | `atomic_t`, `spinlock_t`, sleeping `mutex`, `semaphore`; nonrecursive locks; `preempt_count` gates kernel preemption |
| POSIX | `pthread_mutex`, named/unnamed semaphores, condition variables (always test inside `while`) |
| Java | `synchronized` + `wait`/`notify(All)`; `ReentrantLock` (fair, reentrant); `Semaphore`; named `Condition` |
| Textbook Scope | Silberschatz Ch 6 (Synchronization Tools) + Ch 7 (Synchronization Examples) |

---

<br>

## Self-Check Questions

1. Define a race condition and walk through the producer/consumer `count++` / `count--` example. Why is the symptom timing-dependent?

   > **Answer:** A race condition occurs when two or more threads access shared data concurrently and the **result depends on the interleaving order**. With `count = 5`, `count++` and `count--` each decompose into load → modify → store; an interleaving that lets one thread's store overwrite the other's loses an update, leaving `count = 4` or `count = 6` instead of `5`. Whether the bad interleaving happens depends on scheduling, so the bug appears intermittently — easy to miss in testing.

2. State the three requirements of the Critical-Section Problem. What goes wrong when each one is missing?

   > **Answer:** **Mutual Exclusion** — at most one process in the CS at a time (otherwise race conditions return). **Progress** — if no one is in the CS, a waiting process must be selected without indefinite postponement (otherwise the system can deadlock with the CS empty). **Bounded Waiting** — there is a finite limit on how many other processes can enter before a waiting process is granted entry (otherwise starvation).

3. Compare preemptive and nonpreemptive kernels. Why does a multiprocessor preemptive kernel need more careful synchronization than a single-CPU nonpreemptive kernel?

   > **Answer:** In a **single-CPU nonpreemptive** kernel, no other kernel thread can run until the current one yields, so kernel data races are inherently absent. **Preemptive** kernels allow another thread (potentially on **another core**) to interleave at any moment — so every shared kernel structure needs explicit protection. Disabling interrupts is enough on a uniprocessor but not across cores in SMP.

4. Walk through Peterson's Solution. Why does it satisfy mutual exclusion, and why can it break on modern hardware?

   > **Answer:** Each process Pi sets `flag[i] = true`, then `turn = j`, then waits while `flag[j] && turn == j`. **Mutual exclusion**: if both processes were inside the CS, then `flag[0] = flag[1] = true`, so the only way out for both was `turn == self`, but `turn` holds a single value. Contradiction. **Modern hardware** can reorder the writes `flag[i] = true` and `turn = j`; if `turn = j` is observed first, both processes can pass the entry check. Fix: insert memory barriers between the two writes.

5. Give pseudocode for `test_and_set` and `compare_and_swap`, and show how each implements a lock. Why is CAS strictly more powerful?

   > **Answer:**
   > - `test_and_set(target)` atomically reads `*target` and sets it to `true`, returning the old value. Lock: spin while `test_and_set(&lock)` returns `true`.
   > - `compare_and_swap(value, expected, new_value)` atomically writes `new_value` only if `*value == expected`, returning the original value. Lock: spin while `compare_and_swap(&lock, 0, 1) != 0`.
   > - CAS is more powerful because it works on **arbitrary integers** (not just booleans) and lets you express "update if unchanged since I last read it" — the basis of optimistic concurrency and lock-free data structures.

6. What is an atomic variable, and what is it **not** good for?

   > **Answer:** An atomic variable provides race-free **single-variable** updates (e.g., `atomic_inc(&counter)`) typically built on CAS. It is **not** sufficient for invariants that span multiple variables — e.g., the `count`, `in`, `out` triple in a bounded buffer must be updated together under a single mutex/semaphore, or an observer can see them inconsistent.

7. Compare a spinlock and a sleeping mutex. When is each preferable, and what is the rule of thumb?

   > **Answer:** A **spinlock** busy-loops; a **sleeping mutex** blocks (two context switches). Spinlocks are preferable when the lock is held very briefly — especially on multicore systems where one core can spin while another finishes the CS. Sleeping locks win for long-held locks because the sleeping waiter frees its CPU. **Rule of thumb**: use a spinlock when the expected hold time is **shorter than two context switches**.

8. State the semaphore `wait` and `signal` operations. Why must they be atomic?

   > **Answer:** `wait(S)`: while `S <= 0`, spin; then `S--`. `signal(S)`: `S++`. They must be atomic because the test of `S` and the update must happen as one indivisible step — otherwise two threads can both read `S = 1`, both decrement, and both think they entered, leaving `S = -1` and two threads in the CS.

9. With a semaphore implemented via a waiting queue, what does a **negative** value of `S->value` mean?

   > **Answer:** Its **absolute value equals the number of processes blocked on the semaphore**. Each `wait` decrements first and, if the result is negative, enqueues the caller; each `signal` increments and, if the result is ≤ 0, dequeues and wakes one waiter.

10. List three common semaphore-misuse bugs. Why are they so hard to debug?

    > **Answer:** **(1) Order reversal** — `signal(mutex); CS; wait(mutex)` lets multiple threads into the CS. **(2) Duplicate wait** — `wait(mutex); CS; wait(mutex)` deadlocks at the second wait. **(3) Omitted wait or signal** — leaves the CS unprotected or never wakes anyone. They are hard to debug because they manifest only on specific interleavings and pass typical tests; the right answer is to use a **higher-level construct (monitor)**.

11. What is a monitor, and how does it differ from semaphores? Why are condition variables needed in addition?

    > **Answer:** A monitor is an **ADT that automatically enforces mutual exclusion** on its operations — only one thread can be inside at a time, and the runtime (not the programmer) inserts the lock/unlock. **Condition variables** (`x.wait()`, `x.signal()`) are needed because mutual exclusion alone cannot express "wait until the buffer is non-empty"; `wait` releases the monitor so others can change the state, and `signal` wakes a waiter when they have.

12. Distinguish **signal-and-wait** and **signal-and-continue** policies for monitors. What problem are they solving?

    > **Answer:** When a thread P inside a monitor calls `x.signal()` and another thread Q is waiting on `x`, both want to be the active thread inside the monitor — but only one can be. **Signal-and-wait**: P pauses, Q runs immediately. **Signal-and-continue**: P keeps running until it leaves the monitor, then Q runs. Different languages pick different policies; condition predicates should be re-checked in a `while` loop in either case.

13. Define **deadlock** and give a two-semaphore example.

    > **Answer:** A **deadlock** is a state where every process in a set is waiting for an event that only another process in the set can cause. Example with semaphores S, Q (both 1): P0 does `wait(S); wait(Q);` and P1 does `wait(Q); wait(S);`. If P0 acquires S and is preempted, then P1 acquires Q and tries `wait(S)`, while P0 tries `wait(Q)` — each holds what the other needs.

14. Explain priority inversion with three priorities L < M < H. How does the priority-inheritance protocol fix it, and what real-world incident motivated wider adoption?

    > **Answer:** L holds a lock; H needs the lock and blocks; M becomes runnable and preempts L (since M > L); now H is delayed by M. **Priority inheritance** temporarily raises L to H's priority while L holds the lock H needs, so M cannot preempt L; L finishes and releases the lock; L reverts. **Mars Pathfinder (1997)**: repeated system resets traced to a priority-inversion bug in VxWorks; JPL fixed it remotely by enabling priority inheritance.

15. Solve the Bounded-Buffer Problem with three semaphores. Why does the order of `wait` calls matter?

    > **Answer:** Use `mutex = 1` (binary), `empty = n`, `full = 0` (counting). Producer: `wait(empty); wait(mutex); insert; signal(mutex); signal(full)`. Consumer: `wait(full); wait(mutex); remove; signal(mutex); signal(empty)`. **Order matters** because if a producer reverses to `wait(mutex); wait(empty)`, then when the buffer is full the producer blocks on `empty` *while still holding* `mutex` — the consumer cannot enter to free a slot → **deadlock**.

16. Distinguish the first and second Readers-Writers Problems. Can starvation be eliminated?

    > **Answer:** **First**: readers have priority — a reader may enter even with writers waiting → **writer starvation**. **Second**: writers have priority — once a writer is waiting, no new readers may enter → **reader starvation**. Both variants permit starvation of the disfavored party; eliminating it requires an additional fairness mechanism (e.g., a queue that interleaves readers and writers).

17. Why does the naive semaphore solution to dining-philosophers deadlock? List three remedies.

    > **Answer:** If every philosopher simultaneously executes `wait(chopstick[i])` (her left), all five chopsticks become 0; each then blocks on her right chopstick → **circular wait → deadlock**. Remedies: **(1) at most 4 diners** at a time (limits the cycle); **(2) pick up both chopsticks atomically** in a CS, e.g., the monitor solution; **(3) asymmetric protocol** — odd philosophers pick left first, even pick right first — breaks circular wait.

18. Compare Windows Dispatcher Objects and Critical-Section Objects. When is each used?

    > **Answer:** **Dispatcher Objects** (Mutex, Semaphore, Event, Timer) are kernel objects with signaled/nonsignaled state; acquiring a nonsignaled object blocks the thread in the kernel. **Critical-Section Objects** are user-mode mutexes that **first try a spinlock** in user space, falling back to a kernel mutex only when contention is heavy. Critical-Section is much faster in the common (uncontended) case because it avoids the kernel.

19. Compare Linux's `spinlock_t` and `mutex`. What does it mean that they are **nonrecursive**?

    > **Answer:** `spinlock_t` busy-waits and is suited to very short kernel critical sections; `mutex` is a **sleeping** lock for longer waits. **Nonrecursive** means a thread that already holds the lock will deadlock if it tries to acquire the same lock again — there is no internal "I already hold this" counter. Recursive locking would have to be expressed by structuring code so the same lock is never re-entered.

20. State the three rules every POSIX condition-variable user must follow.

    > **Answer:**
    > **(1)** `pthread_cond_wait()` atomically releases the mutex and blocks; it reacquires the mutex on wakeup.
    > **(2)** `pthread_cond_signal()` does not release the mutex — the signaler must call `pthread_mutex_unlock()` itself.
    > **(3)** Always test the predicate inside a **`while`** loop, not an `if` — the standard permits spurious wakeups, and another thread may have changed state between your wakeup and reacquiring the mutex.

21. Why do Java named `Condition` objects (via `ReentrantLock.newCondition()`) help the bounded-buffer pattern compared to a single anonymous `notify()`?

    > **Answer:** A bounded buffer has two distinct conditions: "not full" (waited on by producers) and "not empty" (waited on by consumers). With a single anonymous `notify()`, the runtime might wake a producer when in fact a consumer's condition has been satisfied (or vice versa) — the awakened thread checks its `while` and goes back to sleep, wasting a wakeup. **Named conditions** let the signaler target the **specific** condition that has been satisfied, eliminating the wasted round-trip.
