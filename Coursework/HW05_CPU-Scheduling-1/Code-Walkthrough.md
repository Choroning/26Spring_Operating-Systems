# HW5 Code Walkthrough — xv6 Priority Scheduler

[Repository Guide](../../README.md) | [English](Code-Walkthrough.md) | [한국어](Code-Walkthrough.ko.md)

> **Last Updated:** 2026-11-16

> **Learning Objectives:**
> 1. Explain which function of which file each hunk of the patch changes and which step of the handout it corresponds to
> 2. Explain the difference between the original and the new scheduler by comparing the removed and added lines
> 3. Trace, line by line, the locks the scheduler holds during the scan and the lock it carries into dispatch
> 4. Explain what the three tests of the test program check and what they do not check

---

## Table of Contents

- [1. File Layout](#1-file-layout)
- [2. Makefile and kernel/defs.h](#2-makefile-and-kerneldefsh)
- [3. kernel/proc.c](#3-kernelprocc)
  - [3.1 allocproc, freeproc, kfork](#31-allocproc-freeproc-kfork)
  - [3.2 scheduler](#32-scheduler)
  - [3.3 sleep](#33-sleep)
  - [3.4 ksetpriority and kgetpriority](#34-ksetpriority-and-kgetpriority)
- [4. kernel/proc.h](#4-kernelproch)
- [5. System Call Registration and Handlers](#5-system-call-registration-and-handlers)
- [6. Test Program and User-Side Files](#6-test-program-and-user-side-files)
- [7. Tracing the Scheduler](#7-tracing-the-scheduler)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. File Layout

> **Try It First:** This document contains the complete code and line-by-line explanations. Solve the problems in the [assignment handout](Original/Assignment.md) yourself before reading on.

The companion document [HW5 Assignment Explanation](Assignment-Explanation.md) summarizes the requirements, the design, the test results, and the answers to the report questions. This document reads the submitted `Solution/priority.patch` (472 lines) from beginning to end in file order. This assignment gave only the steps and code fragments of the handout, without starter code, so the last column of the table lists the step of the handout that each change corresponds to. How to read the unified diff format is the same as in Section 2 of the [HW2 Code Walkthrough](../HW02_Process-2/Code-Walkthrough.md). This patch also has removed lines (`-`).

| Patch lines | File | Change | Step of the handout |
|:-----|:-----|:-----|:-----|
| 1-12 | `Makefile` | Add `_priority_test` to `UPROGS` | Test |
| 13-25 | `kernel/defs.h` | Prototypes of `ksetpriority()` and `kgetpriority()` | 5 |
| 26-53 | `kernel/proc.c` | Priority in `allocproc()`, `freeproc()`, and `kfork()` | 2 |
| 54-127 | `kernel/proc.c` | Rewrite of `scheduler()` | 7 |
| 128-140 | `kernel/proc.c` | Trace output in `sleep()` | Outside the assignment (Week 6 Lab 4) |
| 141-190 | `kernel/proc.c` | `ksetpriority()`, `kgetpriority()` | 5 |
| 191-208 | `kernel/proc.h` | `sched_last` in `struct cpu`, `priority` in `struct proc` | 1, 7 |
| 209-230 | `kernel/syscall.c` | Prototypes and `syscalls[]` entries | 6 |
| 231-240 | `kernel/syscall.h` | System call numbers 22 and 23 | 3 |
| 241-268 | `kernel/sysproc.c` | `sys_setpriority()`, `sys_getpriority()` | 5 |
| 269-449 | `user/priority_test.c` | Test program (new file) | Same as the distributed `Original/priority_test.c` |
| 450-462 | `user/user.h` | Declarations of the two functions | 4 |
| 463-472 | `user/usys.pl` | Entries for the two stubs | 4 |

*Table 1. Structure of the patch and the steps of the handout*

---

<br>

## 2. Makefile and kernel/defs.h

```diff
diff --git a/Makefile b/Makefile
index b262c0a..5b40eca 100755
--- a/Makefile
+++ b/Makefile
@@ -145,6 +145,7 @@ UPROGS=\
 	$U/_logstress\
 	$U/_forphan\
 	$U/_dorphan\
+	$U/_priority_test\
 
 fs.img: mkfs/mkfs README $(UPROGS)
 	mkfs/mkfs fs.img README $(UPROGS)
diff --git a/kernel/defs.h b/kernel/defs.h
index 122d9ca..e16ddc3 100755
--- a/kernel/defs.h
+++ b/kernel/defs.h
@@ -86,6 +86,8 @@ void            proc_mapstacks(pagetable_t);
 pagetable_t     proc_pagetable(struct proc *);
 void            proc_freepagetable(pagetable_t, uint64);
 int             kkill(int);
+int             ksetpriority(int, int);
+int             kgetpriority(int);
 int             killed(struct proc*);
 void            setkilled(struct proc*);
 struct cpu*     mycpu(void);
```

Line 9 puts the test program into the file system image. Lines 21-22 place the prototypes of the two helpers in the `proc.c` section of `defs.h`, right after the existing `kkill()`. The handlers in `sysproc.c` call the two functions through these prototypes.

---

<br>

## 3. kernel/proc.c

### 3.1 allocproc, freeproc, kfork

```diff
diff --git a/kernel/proc.c b/kernel/proc.c
index 22a5401..b0e1c45 100755
--- a/kernel/proc.c
+++ b/kernel/proc.c
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

Each of the three hunks adds one line. Line 34 gives the default priority 10 in `allocproc()` right after the PID and state are set, and line 42 gives 0 in `freeproc()` just before the state returns to `UNUSED`. Line 50 copies the parent's priority to the child in `kfork()` after the name is copied with `safestrcpy()`. These are the positions specified by (a), (b), and (c) of Step 2 in the handout. `allocproc()` holds `p->lock` at this point, so the write of the priority also happens under the lock.

### 3.2 scheduler

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

The first hunk declares the local variable `best` (line 58) and sets this CPU's `sched_last` to 0 when the scheduler starts (line 62). The second hunk removes the original `found` variable and the linear loop from `proc[0]` (lines 70-71), along with the block that switched as soon as it found a RUNNABLE process inside that loop (lines 86-97), the `release()` at the end of the loop (line 106), and the `found == 0` check (line 108).

The comment on lines 72-80 summarizes the new approach. Line 81 clears `best`, and lines 82-84 loop `i` from 0 to `NPROC - 1`, visiting the entry at index `(c->sched_last + 1 + i) % NPROC`. The `acquire()` on line 85 is a line left unchanged from the original code. Lines 98-102 release the previous `best`'s lock and replace `best` if the visited entry is RUNNABLE and either there is no `best` or its priority is strictly lower; the lock just taken is not released. Lines 103-104 release the locks of all other entries.

Lines 110-123 are the dispatch when there is a candidate. Line 115 computes the winner's index as the pointer difference `best - proc` and records it in `sched_last`, and lines 116-118 change the state to RUNNING, set `c->proc`, and call `swtch()`, as the original code did. When that process gives up the CPU through `yield()`, `sleep()`, or `exit()`, `swtch()` returns, and lines 122-123 clear `c->proc` and release the lock. If there is no candidate, lines 124-126 execute `wfi` as in the original code.

### 3.3 sleep

```diff
@@ -556,10 +581,12 @@ sleep(void *chan, struct spinlock *lk)
 
   // Go to sleep.
   p->chan = chan;
+  printf("[sleep]  pid=%d name=%s chan=%p\n", p->pid, p->name, chan);
   p->state = SLEEPING;
 
   sched();
 
+  printf("[wakeup] pid=%d name=%s\n", p->pid, p->name);
   // Tidy up.
   p->chan = 0;
 
```

Lines 132 and 137 print the PID, name, and wait channel just before `sleep()` puts a process to sleep and right after it wakes up. This is code added in Week 6 Lab 4 to observe the state transitions of sleep and wakeup that remained in the working tree and ended up in the patch; it is unrelated to the priority scheduler. The kernel's `printf()` writes directly to the console, so these lines appear interleaved with the test output.

### 3.4 ksetpriority and kgetpriority

```diff
@@ -586,6 +613,49 @@ wakeup(void *chan)
   }
 }
 
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
+
 // Kill the process with the given pid.
 // The victim won't exit until it tries to return
 // to user space (see usertrap() in trap.c).
```

The two functions go after `wakeup()` and before `kkill()`. `ksetpriority()` (lines 148-166) checks the range first and returns -1 for an invalid value without scanning `proc[]`. It then takes each process's lock and compares the PID, the same way as `kkill()`. If the PID matches, it writes the priority, releases the lock, and returns 0. `kgetpriority()` (lines 170-186) searches the same way but copies the value into `prio` before releasing the lock, because reading `p->priority` after the release would let another CPU change the value in between. A slot whose state is `UNUSED` has PID 0, so it never matches a search for a positive PID.

---

<br>

## 4. kernel/proc.h

```diff
diff --git a/kernel/proc.h b/kernel/proc.h
index d021857..b948fbc 100755
--- a/kernel/proc.h
+++ b/kernel/proc.h
@@ -24,6 +24,7 @@ struct cpu {
   struct context context;     // swtch() here to enter scheduler().
   int noff;                   // Depth of push_off() nesting.
   int intena;                 // Were interrupts enabled before push_off()?
+  int sched_last;             // Last-dispatched proc[] index (for RR tie-break)
 };
 
 extern struct cpu cpus[NCPU];
@@ -104,4 +105,5 @@ struct proc {
   struct file *ofile[NOFILE];  // Open files
   struct inode *cwd;           // Current directory
   char name[16];               // Process name (debugging)
+  int priority;                // Scheduling priority (0=highest, 20=lowest)
 };
```

Line 199 adds `sched_last` to `struct cpu`. Being per-CPU, it gives the scheduler of each CPU its own rotation position, and since only the scheduler running on that CPU reads and writes it, no lock is needed. Line 207 appends `priority` to `struct proc`; the comment states the range, with 0 the highest and 20 the lowest.

---

<br>

## 5. System Call Registration and Handlers

```diff
diff --git a/kernel/syscall.c b/kernel/syscall.c
index 076d965..fbf0a85 100755
--- a/kernel/syscall.c
+++ b/kernel/syscall.c
@@ -101,6 +101,8 @@ extern uint64 sys_unlink(void);
 extern uint64 sys_link(void);
 extern uint64 sys_mkdir(void);
 extern uint64 sys_close(void);
+extern uint64 sys_setpriority(void);
+extern uint64 sys_getpriority(void);
 
 // An array mapping syscall numbers from syscall.h
 // to the function that handles the system call.
@@ -126,6 +128,8 @@ static uint64 (*syscalls[])(void) = {
 [SYS_link]    sys_link,
 [SYS_mkdir]   sys_mkdir,
 [SYS_close]   sys_close,
+[SYS_setpriority] sys_setpriority,
+[SYS_getpriority] sys_getpriority,
 };
 
 void
diff --git a/kernel/syscall.h b/kernel/syscall.h
index 3dd926d..8ebab18 100755
--- a/kernel/syscall.h
+++ b/kernel/syscall.h
@@ -20,3 +20,5 @@
 #define SYS_link   19
 #define SYS_mkdir  20
 #define SYS_close  21
+#define SYS_setpriority 22
+#define SYS_getpriority 23
diff --git a/kernel/sysproc.c b/kernel/sysproc.c
index 419e727..141fa11 100755
--- a/kernel/sysproc.c
+++ b/kernel/sysproc.c
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
+
 // return how many clock tick interrupts have occurred
 // since start.
 uint64
```

`kernel/syscall.c` adds the prototypes of the two handlers (lines 217-218) and the `syscalls[]` entries (lines 226-227), and `kernel/syscall.h` adds the numbers 22 and 23 (lines 239-240). The two handlers in `kernel/sysproc.c` (lines 249-264) read the values of the argument registers a0 and a1 with `argint()` and return the helper's return value unchanged. Since the return type is `uint64`, -1 goes into a0 as the value with all bits set, which becomes -1 again as the `int` return value on the user side.

---

<br>

## 6. Test Program and User-Side Files

Lines 269-449 add `user/priority_test.c` as a new file, whose content is the same as the 175 lines of the distributed `Original/priority_test.c`. The three tests check the following.

| Test | What it checks | What it does not check |
|:-----|:-----|:-----|
| 1 | Default 10, value 5 after `setpriority(pid, 5)`, `setpriority` returning -1 for -1 and 21 | Changing another process's priority, non-existent PIDs |
| 2 | Whether a child created by a parent of priority 3 has priority 3 | Reset of reclaimed slots |
| 3 | Whether three children with priorities 19, 10, and 1 all finish | The finishing order (printed but not judged) |

*Table 2. What the three tests of priority_test check*

Each child in Test 3 first inherits the parent's priority 10 through `fork()`, then, once running, changes its own priority with `setpriority(getpid(), ...)` and uses the CPU with `burn(5000000)`, which writes a `volatile` variable. `volatile` keeps the compiler from eliminating the loop. Tests 1 and 2 restore the priority to 10 at the end so as not to affect the next test.

```diff
diff --git a/user/user.h b/user/user.h
index ac84de9..288c694 100755
--- a/user/user.h
+++ b/user/user.h
@@ -24,6 +24,8 @@ int getpid(void);
 char* sys_sbrk(int,int);
 int pause(int);
 int uptime(void);
+int setpriority(int, int);
+int getpriority(int);
 
 // ulib.c
 int stat(const char*, struct stat*);
diff --git a/user/usys.pl b/user/usys.pl
index c5d4c3a..e1f151c 100755
--- a/user/usys.pl
+++ b/user/usys.pl
@@ -42,3 +42,5 @@ entry("getpid");
 entry("sbrk");
 entry("pause");
 entry("uptime");
+entry("setpriority");
+entry("getpriority");
```

Lines 458-459 of `user/user.h` are the C prototypes of the two functions, and lines 471-472 of `user/usys.pl` generate, at build time, stubs that put the number in a7 and execute `ecall`.

---

<br>

## 7. Tracing the Scheduler

Assume `NPROC = 64`, RUNNABLE processes A, B, and C with priorities 10, 1, and 10 at indices 3, 4, and 5, no other RUNNABLE slots, and `sched_last = 4`.

| Round | Scan order | Changes of best | Selected | Next sched_last |
|:-----|:-----|:-----|:-----|:-----|
| 1 | 5, 6, ..., 63, 0, ..., 4 | C(10), B(1) | B | 4 |
| 2 (after B sleeps) | 5, 6, ..., 63, 0, ..., 4 | C(10) | C | 5 |
| 3 | 6, ..., 63, 0, ..., 5 | A(10) | A | 3 |
| 4 | 4, 5, ..., 63, 0, ..., 3 | C(10) | C | 5 |

*Table 3. Scheduling trace of three processes with priorities 10, 1, and 10*

In round 1, C is met first and becomes `best`, but B, met later, has strictly lower priority, so C's lock is released and B replaces it. As long as B is RUNNABLE, B keeps being selected. Once B sleeps and is no longer RUNNABLE, the remaining tied processes A and C are selected alternately, C, A, C, ..., by the rotation of the starting point. A has a smaller index than C, which has the same priority, but the rotation keeps it from starving.

---

<br>

## Summary

| Concept | Key Summary |
|:-----|:-----|
| Patch structure | It consists of changes for Steps 1-7 of the handout, the test program, and two trace-output lines in `sleep()` unrelated to the assignment. |
| Life of a priority | It becomes 10 in `allocproc()`, the parent's value in `kfork()`, and 0 in `freeproc()`, and every write happens under that process's lock. |
| Scheduler changes | The linear loop that switched to the first RUNNABLE found is removed, replaced by one rotated pass that holds the best candidate's lock until dispatch. |
| sched_last | It records the winning index as `best - proc` so that the next scan starts at the following slot, and being per-CPU it needs no lock. |
| Helpers | They check the range first, search for the PID while holding each process's lock, and copy the value to return before releasing the lock. |
| Scope of the tests | Tests 1 and 2 judge values, but Test 3 only sees whether the three children finish and does not judge the order. |

---

<br>

## Self-Check Questions

1. **Removed lines:** The original scheduler kept looping even after running one RUNNABLE process. State how many processes the new scheduler dispatches per scan.

   <details>
   <summary>Answer</summary>

   The original loop switched each time it met a RUNNABLE process while going around `proc[]`, so it could run several processes per pass. The new scheduler scans the whole pass, selects one candidate, and only then dispatches, so it dispatches at most one process per scan.

   </details>

2. **Number of locks:** State the maximum number of process locks this CPU holds at the same time during the scan of Section 3.2, and why.

   <details>
   <summary>Answer</summary>

   At most two. The lock of `best` is held throughout the scan, and line 85 takes one more, the lock of the visited entry. That entry either becomes `best`, releasing the previous `best`'s lock (line 101), or is not a candidate and is released immediately (line 104), so one lock is always dropped before a third would be taken.

   </details>

3. **The local variable in kgetpriority:** Explain why `kgetpriority()` copies the value into `prio` first instead of writing `release(&p->lock); return p->priority;`.

   <details>
   <summary>Answer</summary>

   Reading `p->priority` after releasing the lock could read a value that another CPU changed with `setpriority` in the meantime, or a 0 written by `freeproc()` after the process ended. Copying the value into a local variable while holding the lock returns the priority at the moment the PID was confirmed.

   </details>
