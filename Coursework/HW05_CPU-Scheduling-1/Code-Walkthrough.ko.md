# HW5 코드 총정리 — xv6 우선순위 스케줄러

[레포지토리 안내](../../README.ko.md) | [English](Code-Walkthrough.md) | [한국어](Code-Walkthrough.ko.md)

> **최종 수정일:** 2026-11-16

> **학습 목표:**
> 1. 패치의 각 변경 단위가 어느 파일의 어느 함수를 바꾸는지와 안내문의 어느 단계에 해당하는지 설명할 수 있다
> 2. 지운 줄과 더한 줄을 비교하여 원래 스케줄러와 새 스케줄러의 차이를 설명할 수 있다
> 3. 스케줄러가 훑는 동안 잡고 있는 락과 디스패치할 때 넘기는 락을 줄 단위로 추적할 수 있다
> 4. 테스트 프로그램의 세 테스트가 무엇을 확인하고 무엇을 확인하지 않는지 설명할 수 있다

---

## 목차

- [1. 파일 구성](#1-파일-구성)
- [2. Makefile과 kernel/defs.h](#2-makefile과-kerneldefsh)
- [3. kernel/proc.c](#3-kernelprocc)
  - [3.1 allocproc, freeproc, kfork](#31-allocproc-freeproc-kfork)
  - [3.2 scheduler](#32-scheduler)
  - [3.3 sleep](#33-sleep)
  - [3.4 ksetpriority와 kgetpriority](#34-ksetpriority와-kgetpriority)
- [4. kernel/proc.h](#4-kernelproch)
- [5. 시스템 콜의 등록과 처리 함수](#5-시스템-콜의-등록과-처리-함수)
- [6. 테스트 프로그램과 사용자 쪽 파일](#6-테스트-프로그램과-사용자-쪽-파일)
- [7. 스케줄러의 추적](#7-스케줄러의-추적)
- [요약](#요약)
- [점검 문제](#점검-문제)

---

<br>

## 1. 파일 구성

> **풀어 보기:** 이 문서는 작성한 코드 전체와 줄별 설명을 담고 있다. 먼저 [과제 안내문](Original/Assignment.ko.md)의 문제를 직접 풀어 본 뒤 읽는다.

짝이 되는 문서 [HW5 과제 해설](Assignment-Explanation.ko.md)은 요구사항, 설계, 테스트 결과, 보고서 질문의 답을 정리한다. 이 문서는 제출한 `Solution/priority.patch`(472줄)를 처음부터 끝까지 파일 순서대로 읽는다. 이 과제는 시작 코드 없이 안내문의 단계와 코드 조각만 주었으므로, 표의 마지막 열은 각 변경이 해당하는 안내문의 단계를 적는다. 통합 diff 형식을 읽는 법은 [HW2 코드 총정리](../HW02_Process-2/Code-Walkthrough.ko.md)의 2절과 같다. 이 패치에는 지운 줄(`-`)도 있다.

| 패치 줄 | 파일 | 변경 | 안내문의 단계 |
|:-----|:-----|:-----|:-----|
| 1\~12 | `Makefile` | `UPROGS`에 `_priority_test` 추가 | 테스트 |
| 13\~25 | `kernel/defs.h` | `ksetpriority()`, `kgetpriority()` 원형 | 5 |
| 26\~53 | `kernel/proc.c` | `allocproc()`, `freeproc()`, `kfork()`의 우선순위 | 2 |
| 54\~127 | `kernel/proc.c` | `scheduler()` 재작성 | 7 |
| 128\~140 | `kernel/proc.c` | `sleep()`의 추적 출력 | 과제 밖 (6주차 실습 4) |
| 141\~190 | `kernel/proc.c` | `ksetpriority()`, `kgetpriority()` | 5 |
| 191\~208 | `kernel/proc.h` | `struct cpu`의 `sched_last`, `struct proc`의 `priority` | 1, 7 |
| 209\~230 | `kernel/syscall.c` | 원형과 `syscalls[]` 항목 | 6 |
| 231\~240 | `kernel/syscall.h` | 시스템 콜 번호 22, 23 | 3 |
| 241\~268 | `kernel/sysproc.c` | `sys_setpriority()`, `sys_getpriority()` | 5 |
| 269\~449 | `user/priority_test.c` | 테스트 프로그램 (새 파일) | 배포된 `Original/priority_test.c`와 같음 |
| 450\~462 | `user/user.h` | 두 함수의 선언 | 4 |
| 463\~472 | `user/usys.pl` | 두 스텁의 항목 | 4 |

*표 1. 패치의 구성과 안내문의 단계*

---

<br>

## 2. Makefile과 kernel/defs.h

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

9번 줄은 테스트 프로그램을 파일 시스템 이미지에 넣는다. 21\~22번 줄은 두 보조 함수의 원형을 `defs.h`의 `proc.c` 절, 기존 `kkill()` 바로 뒤에 둔다. `sysproc.c`의 처리 함수가 이 원형을 보고 두 함수를 부른다.

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

세 변경 단위는 각각 한 줄을 더한다. 34번 줄은 `allocproc()`에서 PID와 상태를 정한 직후 기본 우선순위 10을, 42번 줄은 `freeproc()`에서 상태를 `UNUSED`로 돌리기 직전 0을 준다. 50번 줄은 `kfork()`에서 `safestrcpy()`로 이름을 복사한 뒤 부모의 우선순위를 자식에 복사한다. 안내문 2단계의 (a), (b), (c)가 지정한 위치와 같다. `allocproc()`은 이 시점에 `p->lock`을 잡고 있으므로, 우선순위를 쓰는 일도 락 안에서 일어난다.

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

첫 변경 단위는 지역 변수 `best`를 선언하고(58번 줄), 스케줄러가 시작할 때 이 CPU의 `sched_last`를 0으로 둔다(62번 줄). 두 번째 변경 단위는 원래 코드의 `found` 변수와 `proc[0]`부터의 선형 루프(70\~71번 줄)를 지우고, 그 루프 안에서 RUNNABLE을 찾자마자 전환하던 블록(86\~97번 줄)과 루프 끝의 `release()`(106번 줄), `found == 0` 검사(108번 줄)도 지운다.

72\~80번 줄의 주석은 새 방식을 요약한다. 81번 줄에서 `best`를 비우고, 82\~84번 줄은 `i`를 0부터 `NPROC - 1`까지 돌며 첨자 `(c->sched_last + 1 + i) % NPROC`의 항목을 방문한다. 85번 줄의 `acquire()`는 원래 코드에서 그대로 남은 줄이다. 98\~102번 줄은 방문한 항목이 RUNNABLE이고 `best`가 없거나 우선순위가 엄격하게 낮으면 이전 `best`의 락을 풀고 `best`를 바꾸며, 이때 방금 잡은 락은 풀지 않는다. 103\~104번 줄은 나머지 항목의 락을 푼다.

110\~123번 줄은 후보가 있을 때의 디스패치이다. 115번 줄은 포인터의 차이 `best - proc`로 이긴 항목의 첨자를 구해 `sched_last`에 기록하고, 116\~118번 줄은 원래 코드와 같은 방식으로 상태를 RUNNING으로 바꾸고 `c->proc`를 정한 뒤 `swtch()`한다. 그 프로세스가 `yield()`, `sleep()`, `exit()`로 CPU를 돌려주면 `swtch()`가 반환하고, 122\~123번 줄이 `c->proc`를 비우고 락을 푼다. 후보가 없으면 124\~126번 줄이 원래 코드처럼 `wfi`를 실행한다.

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

132번 줄과 137번 줄은 `sleep()`이 프로세스를 재우기 직전과 깨어난 직후에 PID, 이름, 대기 채널을 출력한다. 6주차 실습 4에서 sleep과 wakeup의 상태 전이를 관찰하려고 더한 코드가 작업 트리에 남아 패치에 들어간 것으로, 우선순위 스케줄러와는 상관없다. 커널의 `printf()`는 콘솔에 바로 쓰므로, 이 줄들은 테스트 출력 사이에 섞여 나온다.

### 3.4 ksetpriority와 kgetpriority

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

두 함수는 `wakeup()` 뒤, `kkill()` 앞에 들어간다. `ksetpriority()`(148\~166번 줄)는 범위를 먼저 검사하여 잘못된 값이면 `proc[]`를 훑지 않고 -1을 반환한다. 그다음 `kkill()`과 같은 방식으로 프로세스마다 락을 잡고 PID를 비교한다. PID가 같으면 우선순위를 쓰고 락을 푼 뒤 0을 반환한다. `kgetpriority()`(170\~186번 줄)는 같은 방식으로 찾되, 락을 풀기 전에 값을 `prio`에 옮겨 둔다. 락을 푼 뒤에 `p->priority`를 읽으면 그사이 다른 CPU가 값을 바꿀 수 있기 때문이다. 상태가 `UNUSED`인 슬롯은 PID가 0이므로 양수 PID를 찾는 검색에 걸리지 않는다.

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

199번 줄은 `struct cpu`에 `sched_last`를 더한다. CPU마다 따로 두므로 여러 CPU의 스케줄러가 각자의 회전 위치를 가지며, 그 CPU에서만 실행되는 스케줄러가 읽고 쓰므로 락이 필요 없다. 207번 줄은 `struct proc`의 끝에 `priority`를 더한다. 주석은 0이 가장 높고 20이 가장 낮다는 범위를 적는다.

---

<br>

## 5. 시스템 콜의 등록과 처리 함수

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

`kernel/syscall.c`는 두 처리 함수의 원형(217\~218번 줄)과 `syscalls[]`의 항목(226\~227번 줄)을, `kernel/syscall.h`는 번호 22와 23(239\~240번 줄)을 더한다. `kernel/sysproc.c`의 두 처리 함수(249\~264번 줄)는 `argint()`로 인자 레지스터 a0과 a1의 값을 읽고, 보조 함수의 반환값을 그대로 반환한다. 반환형이 `uint64`이므로 -1은 모든 비트가 1인 값으로 a0에 들어가고, 사용자 쪽 `int` 반환값으로는 다시 -1이 된다.

---

<br>

## 6. 테스트 프로그램과 사용자 쪽 파일

269\~449번 줄은 `user/priority_test.c`를 새 파일로 더하며, 내용은 배포된 `Original/priority_test.c`의 175줄과 같다. 세 테스트가 확인하는 것은 다음과 같다.

| 테스트 | 확인하는 것 | 확인하지 않는 것 |
|:-----|:-----|:-----|
| 1 | 기본값 10, `setpriority(pid, 5)` 뒤의 값 5, -1과 21에 대한 `setpriority`의 -1 반환 | 다른 프로세스의 우선순위 변경, 존재하지 않는 PID |
| 2 | 우선순위 3인 부모가 만든 자식의 우선순위가 3인지 | 회수된 슬롯의 초기화 |
| 3 | 우선순위 19, 10, 1의 세 자식이 모두 끝나는지 | 끝나는 순서 (출력만 하고 판정하지 않음) |

*표 2. priority_test의 세 테스트가 확인하는 것*

테스트 3의 자식은 먼저 `fork()`로 부모의 우선순위 10을 물려받은 뒤, 각자 실행되면서 `setpriority(getpid(), ...)`로 자신의 우선순위를 바꾸고, `volatile` 변수를 쓰는 `burn(5000000)`으로 CPU를 쓴다. `volatile`은 컴파일러가 반복문을 없애지 못하게 한다. 테스트 1과 2는 끝에서 우선순위를 10으로 되돌려 다음 테스트에 영향을 주지 않는다.

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

`user/user.h`의 458\~459번 줄은 두 함수의 C 원형이고, `user/usys.pl`의 471\~472번 줄은 빌드 때 번호를 a7에 넣고 `ecall`하는 스텁을 만든다.

---

<br>

## 7. 스케줄러의 추적

`NPROC = 64`이고, 첨자 3, 4, 5에 우선순위 10, 1, 10인 RUNNABLE 프로세스 A, B, C가 있으며, 나머지 슬롯은 RUNNABLE이 아니고, `sched_last = 4`라고 가정한다.

| 회차 | 훑기의 순서 | 바뀌는 best | 선택 | 다음 sched_last |
|:-----|:-----|:-----|:-----|:-----|
| 1 | 5, 6, ..., 63, 0, ..., 4 | C(10), B(1) | B | 4 |
| 2 (B가 잠든 뒤) | 5, 6, ..., 63, 0, ..., 4 | C(10) | C | 5 |
| 3 | 6, ..., 63, 0, ..., 5 | A(10) | A | 3 |
| 4 | 4, 5, ..., 63, 0, ..., 3 | C(10) | C | 5 |

*표 3. 우선순위 10, 1, 10인 세 프로세스의 스케줄링 추적*

1회차에서는 C를 먼저 만나 `best`가 되지만, 나중에 만난 B의 우선순위가 엄격하게 낮으므로 C의 락을 풀고 B로 바꾼다. B가 RUNNABLE인 동안에는 B가 계속 선택된다. B가 잠들어 RUNNABLE이 아니게 되면 남은 동점 A와 C는 시작점의 회전에 따라 C, A, C, ... 순서로 번갈아 선택된다. A는 우선순위가 같은 C보다 첨자가 작지만 회전 덕분에 굶지 않는다.

---

<br>

## 요약

| 개념 | 핵심 요약 |
|:-----|:-----|
| 패치의 구성 | 안내문의 1\~7단계에 해당하는 변경과 테스트 프로그램, 그리고 과제와 무관한 `sleep()`의 추적 출력 두 줄로 이루어진다. |
| 우선순위의 생애 | `allocproc()`에서 10, `kfork()`에서 부모의 값, `freeproc()`에서 0이 되며, 쓰는 일은 모두 그 프로세스의 락 안에서 일어난다. |
| 스케줄러의 변경 | 처음 찾은 RUNNABLE로 바로 전환하던 선형 루프를 지우고, 회전한 순서로 한 바퀴 훑으며 가장 좋은 후보의 락을 들고 있다가 디스패치한다. |
| sched_last | 이긴 첨자를 `best - proc`로 기록하여 다음 훑기가 그 다음 칸부터 시작하게 하며, CPU마다 따로 있어 락이 필요 없다. |
| 보조 함수 | 범위 검사를 먼저 하고, 프로세스마다 락을 잡은 채 PID를 찾으며, 반환할 값은 락을 풀기 전에 복사한다. |
| 테스트의 범위 | 테스트 1과 2는 값을 판정하지만, 테스트 3은 세 자식이 끝나는지만 보고 순서는 판정하지 않는다. |

---

<br>

## 점검 문제

1. **지운 줄:** 원래 스케줄러는 RUNNABLE 프로세스를 하나 실행한 뒤에도 루프를 계속 돌았다. 새 스케줄러는 한 번 훑을 때 몇 개의 프로세스를 디스패치하는지 쓰라.

   <details>
   <summary>정답</summary>

   원래 루프는 `proc[]`를 한 바퀴 도는 동안 RUNNABLE을 만날 때마다 전환했으므로 한 바퀴에 여러 프로세스를 실행할 수 있었다. 새 스케줄러는 한 바퀴를 모두 훑어 후보 하나를 고른 뒤에 디스패치하므로, 한 번 훑을 때 최대 하나만 디스패치한다.

   </details>

2. **락의 개수:** 3.2절의 훑기 도중에 이 CPU가 동시에 잡고 있는 프로세스 락은 최대 몇 개인지와 그 이유를 쓰라.

   <details>
   <summary>정답</summary>

   최대 두 개이다. `best`의 락은 훑는 내내 잡혀 있고, 85번 줄에서 방문한 항목의 락을 하나 더 잡는다. 그 항목은 `best`가 되면서 이전 `best`의 락이 풀리거나(101번 줄), 후보가 아니어서 바로 풀리므로(104번 줄) 세 번째 락을 잡기 전에 항상 하나가 줄어든다.

   </details>

3. **kgetpriority의 지역 변수:** `kgetpriority()`가 `release(&p->lock); return p->priority;`로 쓰지 않고 `prio`에 먼저 옮기는 이유를 쓰라.

   <details>
   <summary>정답</summary>

   락을 푼 뒤에 `p->priority`를 읽으면, 그사이 다른 CPU가 `setpriority`로 값을 바꾸거나 프로세스가 끝나 `freeproc()`이 0으로 되돌린 값을 읽을 수 있다. 락을 잡은 동안 값을 지역 변수로 복사하면 PID를 확인한 그 순간의 우선순위를 돌려준다.

   </details>
