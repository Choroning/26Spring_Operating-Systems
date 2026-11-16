# HW5 Assignment Explanation — xv6 Priority Scheduler

[Repository Guide](../../README.md) | [English](Assignment-Explanation.md) | [한국어](Assignment-Explanation.ko.md)

> **Last Updated:** 2026-11-16
>
> Operating System Concepts, Silberschatz, Galvin, and Gagne, Ch 5 (5.1-5.3); xv6: a simple, Unix-like teaching operating system, Cox, Kaashoek, and Morris, Ch 7

> **Learning Objectives:**
> 1. Keep a scheduling attribute for each process and manage its value at creation, reclamation, and `fork()`
> 2. Implement system calls that change the state of other processes in line with the locking rules of the process table
> 3. Implement a scheduler that selects the process to run by priority while respecting the locks, and make processes of equal priority take turns fairly
> 4. Explain how the time quantum and the length of the work affect the observed scheduling order
> 5. Explain the starvation problem of priority scheduling, aging, and how to approximate SJF with priorities

---

## Table of Contents

- [1. Assignment Overview](#1-assignment-overview)
- [2. Requirements](#2-requirements)
  - [2.1 Priority Rules and System Calls](#21-priority-rules-and-system-calls)
  - [2.2 Scheduler Behavior](#22-scheduler-behavior)
  - [2.3 Questions and Grading Criteria](#23-questions-and-grading-criteria)
- [3. Modified Files](#3-modified-files)
- [4. Per-Process Priority State](#4-per-process-priority-state)
- [5. The Two System Calls](#5-the-two-system-calls)
- [6. The Scheduler](#6-the-scheduler)
  - [6.1 Selection by Priority and Locks](#61-selection-by-priority-and-locks)
  - [6.2 Round Robin Among Equal Priorities](#62-round-robin-among-equal-priorities)
- [7. Build and Test](#7-build-and-test)
- [8. Answers to the Report Questions](#8-answers-to-the-report-questions)
  - [8.1 Starvation and the Convoy Effect](#81-starvation-and-the-convoy-effect)
  - [8.2 Three Processes of Different Priorities and Round Robin](#82-three-processes-of-different-priorities-and-round-robin)
  - [8.3 Approximating SJF with Priorities](#83-approximating-sjf-with-priorities)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. Assignment Overview

> **Try It First:** This document contains the answers and solutions. Solve the problems in the [assignment handout](Original/Assignment.md) yourself before reading on.

The assignment asks to replace xv6's round-robin scheduler with a priority-based scheduler and to add the system calls `setpriority` and `getpriority`, with which user programs change and query priorities. It builds on the scheduling algorithms of the Week 6 lecture and on the Week 6 lab, which analyzed `swtch.S` and `scheduler()` and observed the interleaving of round robin; the procedure for adding system calls is the same as in the Week 3 assignment (HW2). The original problem is in the [assignment handout](Original/Assignment.md), and the distributed test program is in `Original/priority_test.c` as distributed. The submission consists of `Solution/priority.patch`, the `git diff` of the xv6 working tree, and a report answering three questions; the content of the report is carried over into Sections 3-8 of this document.

```text
Coursework/HW05_CPU-Scheduling-1/
├── Assignment-Explanation.ko.md    # Assignment explanation (Korean)
├── Assignment-Explanation.md       # Assignment explanation (English, this document)
├── Code-Walkthrough.ko.md          # Code walkthrough (Korean)
├── Code-Walkthrough.md             # Code walkthrough (English)
├── Original/
│   ├── Assignment.ko.md            # Assignment handout (Korean)
│   ├── Assignment.md               # Assignment handout (English)
│   └── priority_test.c             # Distributed test program
└── Solution/
    └── priority.patch              # Submitted kernel patch
```

*Figure 1. Directory layout of HW5*

---

<br>

## 2. Requirements

### 2.1 Priority Rules and System Calls

A priority is an integer from 0 (highest) to 20 (lowest) with a default of 10, and processes of equal priority run round-robin. `setpriority(int pid, int priority)` sets the priority of process `pid` and returns 0 on success, or -1 if `pid` is not found or `priority` is outside 0-20. A process can set its own priority or that of its children. `getpriority(int pid)` returns the priority of that process, or -1 if it does not exist.

The assignment gives a seven-step implementation guide. `struct proc` gets a `priority` field (Step 1), which is initialized to 10 in `allocproc()`, reset to 0 in `freeproc()`, and copied from parent to child in `kfork()` (Step 2). The two system calls are registered as numbers 22 and 23 (Step 3), the user-side stubs and declarations are added (Step 4), `sys_setpriority()` and `sys_getpriority()` are implemented (Step 5), and they are put in the dispatch table (Step 6). The hint for Step 5 says to iterate over `proc[]`, declared `extern` in `defs.h`, taking and releasing `p->lock` before and after accessing each process.

### 2.2 Scheduler Behavior

Step 7 is the core of the assignment. Instead of running the first RUNNABLE process found, the scheduler must scan all RUNNABLE processes, select the one with the lowest priority number, and use round robin among several of equal priority. `p->lock` must be taken before checking `p->state` and released when done, and `wfi` is executed when there is no RUNNABLE process. The handout suggests two approaches: two passes, finding the minimum priority and then running the first process with that value, or a single pass that keeps the lock of the best candidate so far and, on finding a better one, releases the old candidate's lock and switches.

For testing, `priority_test.c` is copied to `user/`, added to `UPROGS`, and run. Test 1 checks the default value, the value after `setpriority`, and the rejection of out-of-range values; Test 2 checks inheritance across `fork()`; and Test 3 checks whether three children with priorities 19, 10, and 1 finish in order of priority.

### 2.3 Questions and Grading Criteria

The report must answer three questions. Question 1 asks what happens to a low-priority process when high-priority processes keep arriving and how this relates to the convoy effect of FCFS; Question 2 asks to compare the behavior of 3 `spin` processes with priorities 1, 10, and 19 with the round robin of Lab Exercise 3; and Question 3 asks how to approximate SJF with priority scheduling and what information is needed.

| Item | Weight |
|:-----|-----:|
| `priority` field added and initialized, reset, and inherited | 15% |
| `setpriority` system call (including validation) | 15% |
| `getpriority` system call | 10% |
| `scheduler()` selecting the highest priority | 40% |
| Test program passing all tests | 10% |
| Code style and comments | 10% |

*Table 1. Grading criteria*

---

<br>

## 3. Modified Files

| File | Modification | Step |
|:-----|:-----|:-----|
| `kernel/proc.h` | Added `priority` to `struct proc` and `sched_last` to `struct cpu` | 1 |
| `kernel/proc.c` | Priority init, reset, and inherit; rewrote `scheduler()`; `ksetpriority()` and `kgetpriority()` | 2, 7 |
| `kernel/defs.h` | Prototypes of the two helpers | 5 |
| `kernel/syscall.h` | `SYS_setpriority = 22`, `SYS_getpriority = 23` | 3 |
| `kernel/syscall.c` | Prototypes and `syscalls[]` entries | 6 |
| `kernel/sysproc.c` | `sys_setpriority()`, `sys_getpriority()` | 5 |
| `user/usys.pl`, `user/user.h` | Trap stubs and C prototypes | 4 |
| `Makefile` | Added `$U/_priority_test` to `UPROGS` | Test |

*Table 2. Modified files and the corresponding steps of the handout*

Besides these, the patch contains two `printf()` calls in `sleep()` (lines 132 and 137 of `Solution/priority.patch`) and the test program `user/priority_test.c`. The two `printf()` calls are tracing code added in Week 6 Lab 4 (sleep and wakeup tracing) that remained in the working tree; unrelated to the requirements of the assignment, they print `[sleep]` and `[wakeup]` lines each time a process goes to sleep and wakes up.

---

<br>

## 4. Per-Process Priority State

Steps 1 and 2 require a priority for each process, given the default 10 at creation, 0 at reclamation, and the parent's value at `fork()`. The patch appends `int priority` to `struct proc` (line 207).

```diff
@@ -124,6 +124,7 @@ allocproc(void)
 found:
   p->pid = allocpid();
   p->state = USED;
+  p->priority = 10;                // default scheduling priority
 
   // Allocate a trapframe page.
   if((p->trapframe = (struct trapframe *)kalloc()) == 0){
@@ -168,6 +169,7 @@ freeproc(struct proc *p)
   p->chan = 0;
   p->killed = 0;
   p->xstate = 0;
+  p->priority = 0;
   p->state = UNUSED;
 }
 
@@ -289,6 +291,7 @@ kfork(void)
   np->cwd = idup(p->cwd);
 
   safestrcpy(np->name, p->name, sizeof(p->name));
+  np->priority = p->priority;   // child inherits parent's priority
 
   pid = np->pid;
 
```

`allocproc()` gives the default with `p->priority = 10;` right after obtaining the PID and changing the state to `USED` (line 34). `freeproc()` clears the value with `p->priority = 0;` just before returning the state to `UNUSED` (line 42). `kfork()` passes the parent's priority to the child with `np->priority = p->priority;` right after copying the name (line 50). A child created by a parent of priority 3 therefore also starts at priority 3, which Test 2 checks.

`struct cpu` also gets `int sched_last`, which remembers the `proc[]` index the scheduler last dispatched (line 199). This field is used in Section 6.2 to produce round robin among equal priorities.

---

<br>

## 5. The Two System Calls

Steps 3-6 require registering the two system calls, reading the arguments to find the target process, and rejecting out-of-range priorities. The registration of the numbers (lines 239-240), the prototypes and entries of the dispatch table (lines 217-218 and 226-227), and the user-side declarations and stubs (lines 458-459 and 471-472) follows the same pattern as HW2. The handlers are placed in `sysproc.c`.

```diff
@@ -95,6 +95,23 @@ sys_kill(void)
   return kkill(pid);
 }
 
+uint64
+sys_setpriority(void)
+{
+  int pid, priority;
+  argint(0, &pid);
+  argint(1, &priority);
+  return ksetpriority(pid, priority);
+}
+
+uint64
+sys_getpriority(void)
+{
+  int pid;
+  argint(0, &pid);
+  return kgetpriority(pid);
+}
```

The handlers read the arguments with `argint()` and delegate the actual work to the helper functions `ksetpriority()` and `kgetpriority()` in `proc.c`, just as the existing `sys_kill()` delegates to `kkill()`. Placing the prototypes of the two helpers in `defs.h` (lines 21-22) keeps the code that handles `proc[]` inside `proc.c`.

```diff
+// Set the scheduling priority of the process with pid `pid` to
+// `priority`. Returns 0 on success, -1 if pid not found or the
+// priority is outside [0, 20].
+int
+ksetpriority(int pid, int priority)
+{
+  struct proc *p;
+
+  if(priority < 0 || priority > 20)
+    return -1;
+
+  for(p = proc; p < &proc[NPROC]; p++){
+    acquire(&p->lock);
+    if(p->pid == pid){
+      p->priority = priority;
+      release(&p->lock);
+      return 0;
+    }
+    release(&p->lock);
+  }
+  return -1;
+}
+
+// Return the current scheduling priority of the process with pid
+// `pid`, or -1 if no such process exists.
+int
+kgetpriority(int pid)
+{
+  struct proc *p;
+  int prio;
+
+  for(p = proc; p < &proc[NPROC]; p++){
+    acquire(&p->lock);
+    if(p->pid == pid){
+      prio = p->priority;
+      release(&p->lock);
+      return prio;
+    }
+    release(&p->lock);
+  }
+  return -1;
+}
```

`ksetpriority()` returns -1 immediately if the priority is outside `[0, 20]`, before scanning `proc[]` (lines 153-154). It then takes each process's lock and compares the PID; when found, it changes the priority, releases the lock, and returns 0. `kgetpriority()` searches the same way but first copies the value to return into the local variable `prio` and then releases the lock. If nothing is found by the end, both return -1. Because the priority is read and written under that process's lock, these accesses do not conflict with the scheduler reading the same field.

> **Note:** The handout says that a process can set its own or its children's priority, but this implementation does not check the relationship between the caller and the target, so it can change the priority of any process. The tests change only the caller's own priority, so this difference does not show.

---

<br>

## 6. The Scheduler

### 6.1 Selection by Priority and Locks

Step 7 requires selecting, among the RUNNABLE processes, the one with the lowest priority number while checking the state with the lock held. The patch removes the existing round-robin loop and rewrites it as a single pass that remembers the best candidate so far.

```diff
@@ -425,9 +428,11 @@ void
 scheduler(void)
 {
   struct proc *p;
+  struct proc *best;
   struct cpu *c = mycpu();
 
   c->proc = 0;
+  c->sched_last = 0;
   for(;;){
     // The most recent process to run may have had interrupts
     // turned off; enable them to avoid a deadlock if all
@@ -437,25 +442,45 @@ scheduler(void)
     intr_on();
     intr_off();
 
-    int found = 0;
-    for(p = proc; p < &proc[NPROC]; p++) {
+    // Priority-based selection with round-robin tie-breaking.
+    //
+    // Scan proc[] once in a rotated order starting from the slot
+    // right after the last dispatched one. Track a "best so far"
+    // candidate with its lock held. A new entry replaces `best`
+    // only when its priority is strictly lower; equal priorities
+    // stick with the earlier-visited entry. Because the scan start
+    // rotates, the "earlier" entry among tied candidates shifts
+    // each round, giving round-robin among same-priority procs.
+    best = 0;
+    for(int i = 0; i < NPROC; i++) {
+      int idx = (c->sched_last + 1 + i) % NPROC;
+      p = &proc[idx];
       acquire(&p->lock);
-      if(p->state == RUNNABLE) {
-        // Switch to chosen process.  It is the process's job
-        // to release its lock and then reacquire it
-        // before jumping back to us.
-        p->state = RUNNING;
-        c->proc = p;
-        swtch(&c->context, &p->context);
-
-        // Process is done running for now.
-        // It should have changed its p->state before coming back.
-        c->proc = 0;
-        found = 1;
+      if(p->state == RUNNABLE &&
+         (best == 0 || p->priority < best->priority)) {
+        if(best != 0)
+          release(&best->lock);
+        best = p;                 // keep p->lock held (transfers to best)
+      } else {
+        release(&p->lock);
       }
-      release(&p->lock);
     }
-    if(found == 0) {
+
+    if(best != 0) {
+      // best->lock is still held from the scan.
+      // Switch to chosen process. It is the process's job
+      // to release its lock and then reacquire it
+      // before jumping back to us.
+      c->sched_last = best - proc;
+      best->state = RUNNING;
+      c->proc = best;
+      swtch(&c->context, &best->context);
+
+      // Process is done running for now.
+      // It should have changed its p->state before coming back.
+      c->proc = 0;
+      release(&best->lock);
+    } else {
       // nothing to run; stop running on this core until an interrupt.
       asm volatile("wfi");
     }
```

`best` points to the best candidate found so far, and that candidate's lock remains held. If a visited entry is RUNNABLE and has strictly lower priority (a smaller number) than `best`, the previous candidate's lock is released and the new entry replaces it, with the new entry's lock handed over to `best` instead of being released (lines 98-102). Any other entry has its lock released immediately (line 104). After the scan, with the winner's lock still held, its state is changed to RUNNING and the switch is made with `swtch()` (lines 115-118). When the process gives up the CPU and control returns to the scheduler, the lock is released (line 123). This preserves the xv6 locking invariant that the scheduler holds a process's lock when switching to it. If there is no RUNNABLE process, the core stops with `wfi` until an interrupt arrives.

> **Caution:** During a scan, up to two locks are held (the lock of `best` and that of the entry being visited). The schedulers of other CPUs take locks the same way, so on multiple CPUs it is theoretically possible for two schedulers to each wait for a lock the other holds. Always scanning from `proc[0]`, as in the handout's approach, makes every scheduler take the locks in the same order and rules out such circular waiting, but rotating the starting point makes the order differ from CPU to CPU. The runs in this assignment use one CPU (`CPUS=1`), so such contention does not arise.

### 6.2 Round Robin Among Equal Priorities

The specification requires round robin among processes of equal priority. Since a new candidate replaces `best` only when its priority is strictly lower, the entry visited first wins a tie. If the scan always started from `proc[0]`, then of three children that all start at priority 10, the one with the lowest index would be re-selected every time and its siblings would starve. This problem was actually observed during development and was fixed by adding `sched_last`.

On lines 82-84, the patch starts the scan at `(c->sched_last + 1) % NPROC` and goes around once, and at dispatch on line 115 it records the winner's index in `c->sched_last`. The next scan then starts at the slot right after the process just run, so the first-visited entry among tied candidates shifts by one slot each time, and processes of equal priority are selected in turn. This is what makes the tie-breaking genuine round robin.

---

<br>

## 7. Build and Test

The patch adds `user/priority_test.c` as a new file and puts `$U/_priority_test` in `UPROGS` (line 9), so xv6 builds directly after the patch is applied to the `Coursework/xv6-riscv` submodule, which points to the base commit (`5474d4b`). After rebuilding the kernel and running `priority_test` under QEMU, all three subtests pass: `setpriority` and `getpriority` with range checks (Test 1), priority inheritance across `fork()` (Test 2), and completion of three children with priorities 1, 10, and 19 (Test 3).

```bash
git submodule update --init Coursework/xv6-riscv
cd Coursework/xv6-riscv
git apply ../HW05_CPU-Scheduling-1/Solution/priority.patch
make clean && make CPUS=1 qemu
# In the xv6 shell
$ priority_test
```

The output of running the patched xv6 on one CPU, with the `[sleep]` and `[wakeup]` lines removed, is as follows.

```text
=== Priority Scheduler Test ===

--- Test 1: setpriority/getpriority ---
PID 3: default priority = 10
PID 3: after setpriority(3, 5), priority = 5
setpriority with invalid priority (-1): returned -1 (OK)
setpriority with invalid priority (21): returned -1 (OK)
Test 1 PASSED

--- Test 2: Priority inheritance via fork ---
Parent priority = 3
Child priority = 3
Test 2 PASSED

--- Test 3: High-priority process runs first ---
[LOW  prio=19] finished
[MED  prio=10] finished
[HIGH prio=1] finished
Test 3 PASSED

All tests passed!
```

The PID is 3 rather than the 4 of the handout's example because of the order of processes at run time, which does not affect the result. In Test 3, the three children finished in their creation order, LOW, MED, HIGH, rather than the expected HIGH, MED, LOW. Test 3 only waits for the three children and does not check the order, so it prints `PASSED`. The reason for this order is explained in Section 8.2.

---

<br>

## 8. Answers to the Report Questions

### 8.1 Starvation and the Convoy Effect

Question 1 asks what happens to a low-priority process when high-priority processes keep arriving and how that relates to the convoy effect of FCFS.

Under a strict priority scheduler, a low-priority process can be starved indefinitely (**starvation**) if higher-priority processes keep arriving: the scan for the best candidate always picks one of them first, and the low-priority process is never dispatched. This is the priority-scheduling analogue of the **convoy effect** of FCFS: in both cases some process waits an unbounded time for the CPU, but the cause differs. FCFS penalizes circumstance (the arrival order behind a long burst), while priority scheduling penalizes a property (the priority value), so the disadvantage persists across workloads. The classical remedy is **aging**: the priority of a waiting process is gradually raised over time, guaranteeing that any starved process eventually climbs to the top of the queue.

### 8.2 Three Processes of Different Priorities and Round Robin

Question 2 asks to compare the scheduling behavior of 3 `spin` processes with priorities 1, 10, and 19 with the round robin of Lab Exercise 3.

In Week 6 Lab Exercise 3, three `spin` processes under round robin each got a tick in turn and interleaved their output nearly evenly. The equivalent experiment here is Test 3 of `priority_test`: it forks three CPU-bound children and gives them priorities 1, 10, and 19 with `setpriority`, the same workload structure as three prioritized `spin` processes. Under the new scheduler, HIGH becomes the strictly best candidate once it has lowered its own priority value to 1 and monopolizes the CPU until it exits; MED then runs to completion, and LOW runs last.

Two conditions must hold for this ordering to be visible in the `finished` lines: the work must be long enough to cross a time quantum boundary (about 100 ms, set in `clockintr`), and the tie-break must rotate. With the distributed `burn(5000000)` (5 × 10⁶ iterations) on `CPUS=1`, each child completes within one quantum, so the order is simply the creation order, LOW, MED, HIGH; the output in Section 7 is this case. Raising the iteration count to 5 × 10⁷ exposes preemption, and the expected HIGH, MED, LOW order appears. Running a copy of the test program with only the iteration count changed, on the same kernel, gives the following.

```text
--- Test 3: High-priority process runs first ---
[HIGH prio=1] finished
[MED  prio=10] finished
[LOW  prio=19] finished
Test 3 PASSED
```

The difference from round robin is qualitative: round robin interleaves forever, whereas priority scheduling serializes execution by priority class.

> **Note:** That the scheduler gets no chance to choose again when work finishes within one quantum holds for round robin as well. The scheduling order is determined not only by the policy but also by when preemption happens, that is, by the relation between the time quantum and the length of the work. The observation that the expected order appeared only after the work was lengthened illustrates this relation.

### 8.3 Approximating SJF with Priorities

Question 3 asks how to approximate SJF, which is optimal for average waiting time, with priority scheduling and what information is needed.

SJF is already a priority scheduler in disguise, with the priority equal to (the inverse of) the predicted next CPU burst. To approximate it, the kernel maintains a per-process running estimate of the next burst length and maps it onto the 0-20 priority scale before each scheduling decision. The estimate comes from observed history, typically through the following exponential average, where $t_n$ is the most recently measured CPU burst and $\tau_n$ is the prediction up to that point.

$$
\tau_{n+1} = \alpha t_n + (1 - \alpha) \tau_n, \qquad 0 \le \alpha \le 1
$$

Measuring requires recording, at every context switch, how long the process ran before yielding or blocking; only CPU bursts count, not time spent waiting for I/O. The information needed is therefore a per-process burst history stored inside `struct proc` alongside the `priority` field. Pure SJF is brittle, though: it relies on the estimates tracking reality, and I/O-bound processes with very short bursts could monopolize the CPU. That is why real systems do not use SJF alone but combine an SJF-like bias with aging and multilevel feedback queues.

---

<br>

## Summary

| Concept | Key Summary |
|:-----|:-----|
| Priority state | `allocproc()` initializes it to 10, `freeproc()` resets it to 0, and `kfork()` copies it to the child, while `sched_last` in `struct cpu` remembers the last dispatched index. |
| System calls | The handlers read the arguments with `argint()` and delegate to `ksetpriority()` and `kgetpriority()` in `proc.c`, which search for the PID under each process's lock and return -1 on failure. |
| Selection and locks | A single scan releases the previous candidate's lock and keeps the new candidate's lock whenever it meets a RUNNABLE entry of strictly lower priority, and switches with `swtch()` while holding the winner's lock. |
| Round robin on ties | Rotating the start of the scan to `(sched_last + 1) % NPROC` changes which of the equal-priority candidates is visited first each time. |
| Test results | All three tests pass, but with the distributed iteration count each child finishes within one quantum, so the order in Test 3 is the creation order; increasing the count tenfold makes the priority order appear. |
| Starvation and aging | Strict priority can starve low-priority processes, which differs from the FCFS convoy effect in being caused by a property, and aging remedies it. |
| Approximating SJF | The next burst is predicted by exponential averaging and mapped to a priority, which requires a history of CPU bursts measured at every context switch. |

---

<br>

## Self-Check Questions

1. **Stuck ties:** If the scan always starts at `proc[0]` and compares with `p->priority < best->priority`, state what happens when three processes of priority 10 are at indices 3, 4, and 5.

   <details>
   <summary>Answer</summary>

   The process at index 3 is visited first and becomes `best`, and 4 and 5 cannot replace it because their priority is not strictly lower. As long as that process is RUNNABLE, index 3 is selected every time, and 4 and 5 starve.

   </details>

2. **Start of the scan:** With `NPROC = 64` and `proc[63]` as the last dispatched process, write the first and last indices visited by the next scan.

   <details>
   <summary>Answer</summary>

   Since `c->sched_last = 63`, the first index is `(63 + 1 + 0) % 64 = 0` and the last (i = 63) is `(63 + 1 + 63) % 64 = 63`. The process just run is visited last, so on a tie another process is picked first.

   </details>

3. **Exponential average:** With $\alpha = 0.5$ and an initial prediction $\tau_0 = 10$ ms, compute $\tau_2$ after actual bursts of 6 ms and 4 ms are measured.

   <details>
   <summary>Answer</summary>

   $\tau_1 = 0.5 \times 6 + 0.5 \times 10 = 8$ ms, and $\tau_2 = 0.5 \times 4 + 0.5 \times 8 = 6$ ms.

   </details>

4. **Order in Test 3:** Explain why the scheduler cannot be called wrong even though the distributed Test 3 finishes in the order LOW, MED, HIGH on `CPUS=1`.

   <details>
   <summary>Answer</summary>

   The 5 × 10⁶ iterations of each child are short enough to finish within one time quantum. The child that runs first therefore has no chance to be preempted by the timer interrupt before it changes its priority and finishes its work, and the scheduler gets to choose another child only after it has ended. Increasing the count to 5 × 10⁷ makes the scheduler choose again every quantum, and the HIGH, MED, LOW order appears, so the priority selection works correctly.

   </details>
