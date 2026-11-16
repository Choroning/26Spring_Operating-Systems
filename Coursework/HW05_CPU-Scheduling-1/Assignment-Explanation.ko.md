# HW5 과제 해설 — xv6 우선순위 스케줄러

[레포지토리 안내](../../README.ko.md) | [English](Assignment-Explanation.md) | [한국어](Assignment-Explanation.ko.md)

> **최종 수정일:** 2026-11-16
>
> Operating System Concepts, Silberschatz, Galvin, and Gagne, Ch 5 (5.1-5.3); xv6: a simple, Unix-like teaching operating system, Cox, Kaashoek, and Morris, Ch 7

> **학습 목표:**
> 1. 프로세스마다 스케줄링 속성을 두고 생성, 회수, `fork()`에서 그 값을 관리할 수 있다
> 2. 다른 프로세스의 상태를 바꾸는 시스템 콜을 프로세스 테이블의 락 규칙에 맞게 구현할 수 있다
> 3. 락을 지키면서 실행할 프로세스를 우선순위로 고르는 스케줄러를 구현하고, 같은 우선순위끼리 공정하게 돌아가게 할 수 있다
> 4. 타임 퀀텀과 작업 길이가 관찰되는 스케줄링 순서에 미치는 영향을 설명할 수 있다
> 5. 우선순위 스케줄링의 기아 문제와 노화, 우선순위로 SJF를 근사하는 방법을 설명할 수 있다

---

## 목차

- [1. 과제 개요](#1-과제-개요)
- [2. 요구사항](#2-요구사항)
  - [2.1 우선순위 규칙과 시스템 콜](#21-우선순위-규칙과-시스템-콜)
  - [2.2 스케줄러의 동작](#22-스케줄러의-동작)
  - [2.3 질문과 채점 기준](#23-질문과-채점-기준)
- [3. 변경한 파일](#3-변경한-파일)
- [4. 프로세스의 우선순위 상태](#4-프로세스의-우선순위-상태)
- [5. 두 시스템 콜](#5-두-시스템-콜)
- [6. 스케줄러](#6-스케줄러)
  - [6.1 우선순위에 따른 선택과 락](#61-우선순위에-따른-선택과-락)
  - [6.2 같은 우선순위 사이의 라운드 로빈](#62-같은-우선순위-사이의-라운드-로빈)
- [7. 빌드와 테스트](#7-빌드와-테스트)
- [8. 보고서 질문의 답](#8-보고서-질문의-답)
  - [8.1 기아와 호위 효과](#81-기아와-호위-효과)
  - [8.2 우선순위가 다른 세 프로세스와 라운드 로빈](#82-우선순위가-다른-세-프로세스와-라운드-로빈)
  - [8.3 우선순위로 SJF 근사하기](#83-우선순위로-sjf-근사하기)
- [요약](#요약)
- [점검 문제](#점검-문제)

---

<br>

## 1. 과제 개요

> **풀어 보기:** 이 문서는 정답과 풀이를 담고 있다. 먼저 [과제 안내문](Original/Assignment.ko.md)의 문제를 직접 풀어 본 뒤 읽는다.

과제는 xv6의 라운드 로빈 스케줄러를 우선순위 기반 스케줄러로 바꾸고, 사용자 프로그램이 우선순위를 바꾸고 조회하는 시스템 콜 `setpriority`와 `getpriority`를 더하는 문제이다. 6주차 이론의 스케줄링 알고리즘과, 6주차 실습에서 `swtch.S`와 `scheduler()`를 분석하고 라운드 로빈의 번갈아 실행을 관찰한 내용이 바탕이 되며, 시스템 콜을 더하는 절차는 3주차 과제(HW2)와 같다. 문제의 원문은 [과제 안내문](Original/Assignment.ko.md)에 있고, 배포된 테스트 프로그램은 `Original/priority_test.c`에 원본 그대로 있다. 제출물은 xv6 작업 트리의 `git diff`인 `Solution/priority.patch`와 세 질문에 답한 보고서이며, 보고서의 내용은 이 문서의 3\~8절에 옮겼다.

```text
Coursework/HW05_CPU-Scheduling-1/
├── Assignment-Explanation.ko.md    # 과제 해설 (한국어, 이 문서)
├── Assignment-Explanation.md       # 과제 해설 (영어)
├── Code-Walkthrough.ko.md          # 코드 총정리 (한국어)
├── Code-Walkthrough.md             # 코드 총정리 (영어)
├── Original/
│   ├── Assignment.ko.md            # 과제 안내문 (한국어)
│   ├── Assignment.md               # 과제 안내문 (영어)
│   └── priority_test.c             # 배포된 테스트 프로그램
└── Solution/
    └── priority.patch              # 제출한 커널 패치
```

*그림 1. HW5 과제의 디렉토리 구성*

---

<br>

## 2. 요구사항

### 2.1 우선순위 규칙과 시스템 콜

우선순위는 0(가장 높음)부터 20(가장 낮음)까지의 정수이고 기본값은 10이며, 우선순위가 같은 프로세스끼리는 라운드 로빈으로 실행한다. `setpriority(int pid, int priority)`는 프로세스 `pid`의 우선순위를 정하고 성공하면 0을, `pid`가 없거나 `priority`가 0\~20을 벗어나면 -1을 반환한다. 프로세스는 자신이나 자식의 우선순위를 정할 수 있다. `getpriority(int pid)`는 그 프로세스의 우선순위를 반환하고, 없으면 -1을 반환한다.

과제는 일곱 단계의 구현 안내를 준다. `struct proc`에 `priority` 필드를 두고(1단계), `allocproc()`에서 10으로 초기화하고 `freeproc()`에서 0으로 되돌리며 `kfork()`에서 부모의 값을 자식에게 복사한다(2단계). 두 시스템 콜을 22번과 23번으로 등록하고(3단계), 사용자 쪽 스텁과 선언을 더하며(4단계), `sys_setpriority()`와 `sys_getpriority()`를 구현하고(5단계), 분배 테이블에 넣는다(6단계). 5단계의 힌트는 `defs.h`에 `extern`으로 선언된 `proc[]`를 돌며, 각 프로세스에 접근하기 전후로 `p->lock`을 잡고 풀라고 한다.

### 2.2 스케줄러의 동작

7단계가 과제의 중심이다. 스케줄러는 처음 찾은 RUNNABLE 프로세스를 실행하는 대신, 모든 RUNNABLE 프로세스를 훑어 우선순위 숫자가 가장 작은 것을 고르고, 같은 우선순위가 여럿이면 그들 사이에서 라운드 로빈을 써야 한다. `p->state`를 검사하기 전에 `p->lock`을 잡고 다 쓴 뒤 풀어야 하며, RUNNABLE 프로세스가 없으면 `wfi`를 실행한다. 안내문은 두 가지 방법을 제시한다. 두 번 훑어 최소 우선순위를 찾은 뒤 그 값을 가진 첫 프로세스를 실행하는 방법과, 한 번 훑으며 지금까지 가장 좋은 후보의 락을 잡아 두고 더 좋은 후보를 찾으면 이전 후보의 락을 풀고 바꾸는 방법이다.

테스트는 `priority_test.c`를 `user/`에 복사하고 `UPROGS`에 더한 뒤 실행한다. 테스트 1은 기본값, `setpriority` 뒤의 값, 범위를 벗어난 값의 거부를, 테스트 2는 `fork()`에서의 상속을, 테스트 3은 우선순위 19, 10, 1의 세 자식이 우선순위가 높은 순서로 끝나는지를 본다.

### 2.3 질문과 채점 기준

보고서는 세 질문에 답해야 한다. 질문 1은 우선순위가 높은 프로세스가 계속 도착할 때 낮은 프로세스가 어떻게 되는지와 FCFS의 호위 효과와의 관계를, 질문 2는 우선순위 1, 10, 19의 `spin` 프로세스 3개의 동작을 실습 연습 3의 라운드 로빈과 비교하라고, 질문 3은 우선순위 스케줄링으로 SJF를 근사하는 방법과 필요한 정보를 묻는다.

| 항목 | 비중 |
|:-----|-----:|
| `priority` 필드의 추가와 초기화, 재설정, 상속 | 15% |
| `setpriority` 시스템 콜 (검증 포함) | 15% |
| `getpriority` 시스템 콜 | 10% |
| `scheduler()`의 최고 우선순위 선택 | 40% |
| 테스트 프로그램의 모든 테스트 통과 | 10% |
| 코드 스타일과 주석 | 10% |

*표 1. 채점 기준*

---

<br>

## 3. 변경한 파일

| 파일 | 변경 | 안내문의 단계 |
|:-----|:-----|:-----|
| `kernel/proc.h` | `struct proc`에 `priority`, `struct cpu`에 `sched_last` 추가 | 1 |
| `kernel/proc.c` | 우선순위의 초기화, 재설정, 상속, `scheduler()` 재작성, `ksetpriority()`와 `kgetpriority()` | 2, 7 |
| `kernel/defs.h` | 두 보조 함수의 원형 | 5 |
| `kernel/syscall.h` | `SYS_setpriority = 22`, `SYS_getpriority = 23` | 3 |
| `kernel/syscall.c` | 원형과 `syscalls[]` 항목 | 6 |
| `kernel/sysproc.c` | `sys_setpriority()`, `sys_getpriority()` | 5 |
| `user/usys.pl`, `user/user.h` | 트랩 스텁과 C 원형 | 4 |
| `Makefile` | `UPROGS`에 `$U/_priority_test` 추가 | 테스트 |

*표 2. 변경한 파일과 안내문의 단계*

패치에는 이 밖에 `sleep()`의 두 `printf()`(`Solution/priority.patch`의 132, 137번 줄)와 테스트 프로그램 `user/priority_test.c`가 들어 있다. 두 `printf()`는 6주차 실습 4(sleep과 wakeup 추적)에서 더한 추적 코드가 작업 트리에 남은 것으로, 과제의 요구와는 상관없이 프로세스가 잠들고 깨어날 때마다 `[sleep]`과 `[wakeup]` 줄을 출력한다.

---

<br>

## 4. 프로세스의 우선순위 상태

1, 2단계는 프로세스마다 우선순위를 두고, 생성할 때 기본값 10, 회수할 때 0, `fork()`할 때 부모의 값을 주라고 요구한다. 패치는 `struct proc`의 끝에 `int priority`를 더한다(207번 줄).

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

`allocproc()`는 PID를 받고 상태를 `USED`로 바꾼 직후 `p->priority = 10;`으로 기본값을 준다(34번 줄). `freeproc()`은 상태를 `UNUSED`로 돌리기 직전에 `p->priority = 0;`으로 값을 지운다(42번 줄). `kfork()`는 이름을 복사한 바로 뒤에 `np->priority = p->priority;`로 부모의 우선순위를 자식에게 물려준다(50번 줄). 그래서 우선순위 3인 부모가 만든 자식도 우선순위 3에서 시작하며, 테스트 2가 이를 확인한다.

`struct cpu`에는 스케줄러가 마지막으로 디스패치한 `proc[]`의 첨자를 기억하는 `int sched_last`도 더했다(199번 줄). 이 필드는 6.2절에서 같은 우선순위 사이의 라운드 로빈을 만들기 위해 쓰인다.

---

<br>

## 5. 두 시스템 콜

3\~6단계는 두 시스템 콜을 등록하고, 인자를 읽어 대상 프로세스를 찾고, 범위를 벗어난 우선순위를 거부하라고 요구한다. 번호(239\~240번 줄), 분배 테이블의 원형과 항목(217\~218, 226\~227번 줄), 사용자 쪽 선언과 스텁(458\~459, 471\~472번 줄)의 등록은 HW2와 같은 방식이다. 처리 함수는 `sysproc.c`에 둔다.

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

처리 함수는 `argint()`로 인자를 읽고, 실제 일은 `proc.c`의 보조 함수 `ksetpriority()`와 `kgetpriority()`에 맡긴다. 기존의 `sys_kill()`이 `kkill()`에 맡기는 방식과 같다. 두 보조 함수의 원형을 `defs.h`에 두면(21\~22번 줄) `proc[]`를 다루는 코드가 `proc.c` 안에 머문다.

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

`ksetpriority()`는 `proc[]`를 훑기 전에 우선순위가 `[0, 20]`을 벗어나면 곧바로 -1을 반환한다(153\~154번 줄). 그다음 각 프로세스의 락을 잡고 PID를 비교하여, 찾으면 우선순위를 바꾸고 락을 푼 뒤 0을 반환한다. `kgetpriority()`도 같은 방식으로 찾되, 락을 푼 뒤에 반환할 값을 지역 변수 `prio`에 먼저 옮겨 둔다. 끝까지 찾지 못하면 둘 다 -1을 반환한다. 우선순위를 읽고 쓰는 일을 그 프로세스의 락 안에서 하므로, 스케줄러가 같은 필드를 읽는 것과 충돌하지 않는다.

> **참고:** 안내문은 프로세스가 자신이나 자식의 우선순위를 정할 수 있다고 하지만, 이 구현은 호출한 프로세스와 대상의 관계를 검사하지 않으므로 어떤 프로세스의 우선순위든 바꿀 수 있다. 테스트는 자기 자신의 우선순위만 바꾸므로 이 차이가 드러나지 않는다.

---

<br>

## 6. 스케줄러

### 6.1 우선순위에 따른 선택과 락

7단계는 RUNNABLE 프로세스 가운데 우선순위 숫자가 가장 작은 것을 고르되, 락을 잡은 채로 상태를 검사하라고 요구한다. 패치는 기존의 라운드 로빈 루프를 지우고, 한 번 훑으며 지금까지 가장 좋은 후보를 기억하는 방식으로 다시 쓴다.

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

`best`는 지금까지 찾은 가장 좋은 후보를 가리키며, 그 후보의 락은 잡힌 채로 남는다. 방문한 항목이 RUNNABLE이고 `best`보다 우선순위가 엄격하게 낮으면(숫자가 작으면) 이전 후보의 락을 풀고 새 항목으로 바꾸며, 이때 새 항목의 락은 풀지 않고 `best`에게 넘긴다(98\~102번 줄). 그렇지 않은 항목은 바로 락을 푼다(104번 줄). 훑기가 끝나면 이긴 프로세스의 락이 여전히 잡혀 있는 상태에서 상태를 RUNNING으로 바꾸고 `swtch()`로 전환한다(115\~118번 줄). 프로세스가 CPU를 돌려주어 스케줄러로 돌아오면 그 락을 푼다(123번 줄). 이렇게 하면 "스케줄러가 프로세스로 전환할 때 그 프로세스의 락을 잡고 있다"는 xv6의 락 불변식이 지켜진다. RUNNABLE 프로세스가 없으면 인터럽트가 올 때까지 `wfi`로 코어를 멈춘다.

> **주의:** 한 번 훑는 동안 락은 최대 두 개(`best`의 락과 지금 방문한 항목의 락)까지 잡힌다. 다른 CPU의 스케줄러도 같은 방식으로 락을 잡으므로, 여러 CPU에서 두 스케줄러가 서로 상대가 잡은 락을 기다리는 일이 이론적으로는 가능하다. 안내문의 방법처럼 언제나 `proc[0]`부터 훑으면 모든 스케줄러가 같은 순서로 락을 잡으므로 이런 순환 대기가 생기지 않지만, 시작점을 돌리면 CPU마다 순서가 달라진다. 이 과제의 실행은 CPU 하나(`CPUS=1`)로 하므로 그런 경합이 생기지 않는다.

### 6.2 같은 우선순위 사이의 라운드 로빈

명세는 우선순위가 같은 프로세스끼리 라운드 로빈으로 실행하라고 요구한다. 새 후보는 우선순위가 엄격하게 낮을 때만 `best`를 바꾸므로, 동점이면 먼저 방문한 항목이 남는다. 훑기를 언제나 `proc[0]`부터 시작하면, 처음에 모두 우선순위 10에서 시작하는 세 자식 가운데 첨자가 가장 작은 프로세스만 계속 다시 뽑히고 형제들은 굶는다. 개발 중에 실제로 이 문제가 관찰되었고, `sched_last`를 더해 해결했다.

패치는 82\~84번 줄에서 훑기를 `(c->sched_last + 1) % NPROC`부터 시작하여 한 바퀴 돌고, 디스패치할 때 115번 줄에서 이긴 프로세스의 첨자를 `c->sched_last`에 기록한다. 그러면 다음 훑기는 방금 실행한 프로세스의 바로 다음 칸부터 시작하므로, 동점인 후보들 가운데 먼저 방문하는 쪽이 매번 한 칸씩 옮겨 가고, 같은 우선순위의 프로세스들이 차례로 선택된다. 이 회전 덕분에 동점 처리가 실제로 라운드 로빈이 된다.

---

<br>

## 7. 빌드와 테스트

패치는 `user/priority_test.c`를 새 파일로 더하고 `UPROGS`에 `$U/_priority_test`를 넣으므로(9번 줄), 기준 커밋(`5474d4b`)을 가리키는 `Coursework/xv6-riscv` 서브모듈에 적용한 뒤 바로 빌드한다. 커널을 다시 빌드하고 QEMU에서 `priority_test`를 실행하면 세 테스트가 모두 통과한다. 이는 범위 검사를 포함한 `setpriority`와 `getpriority`(테스트 1), `fork()`에서의 우선순위 상속(테스트 2), 우선순위 1, 10, 19인 세 자식의 완료(테스트 3)를 뜻한다.

```bash
git submodule update --init Coursework/xv6-riscv
cd Coursework/xv6-riscv
git apply ../HW05_CPU-Scheduling-1/Solution/priority.patch
make clean && make CPUS=1 qemu
# xv6 셸에서
$ priority_test
```

패치를 적용한 xv6를 CPU 하나로 실행한 출력에서 `[sleep]`과 `[wakeup]` 줄을 뺀 결과는 다음과 같다.

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

PID가 안내문의 예시(4)와 다른 3인 것은 실행할 때의 프로세스 순서 때문이며 결과와 상관없다. 테스트 3에서 세 자식은 기대한 HIGH, MED, LOW가 아니라 만들어진 순서인 LOW, MED, HIGH로 끝났다. 테스트 3은 세 자식을 기다리기만 하고 순서를 검사하지 않으므로 `PASSED`를 출력한다. 이 순서가 나온 이유는 8.2절에서 설명한다.

---

<br>

## 8. 보고서 질문의 답

### 8.1 기아와 호위 효과

질문 1은 우선순위가 높은 프로세스가 계속 도착할 때 낮은 프로세스가 어떻게 되는지와, 그것이 FCFS의 호위 효과와 어떤 관계인지를 묻는다.

엄격한 우선순위 스케줄러에서는 우선순위가 높은 프로세스가 계속 도착하면 낮은 프로세스가 끝없이 **기아(starvation)** 상태에 빠질 수 있다. 가장 좋은 후보를 고르는 훑기가 매번 높은 프로세스 가운데 하나를 먼저 고르므로, 낮은 프로세스는 디스패치되지 않는다. 이는 FCFS의 **호위 효과(convoy effect)** 에 대응하는 우선순위 스케줄링의 현상이다. 두 경우 모두 어떤 프로세스가 CPU를 한없이 기다리지만 원인이 다르다. FCFS는 상황(긴 버스트 뒤에 도착한 순서)으로 불리해지고, 우선순위 스케줄링은 속성(우선순위 값)으로 불리해지므로, 그 불리함이 작업 부하가 바뀌어도 이어진다. 고전적인 해결책은 **노화(aging)** 이다. 기다리는 프로세스의 우선순위를 시간이 지남에 따라 조금씩 올려, 굶던 프로세스도 결국 큐의 맨 위에 오르게 보장한다.

### 8.2 우선순위가 다른 세 프로세스와 라운드 로빈

질문 2는 우선순위 1, 10, 19인 `spin` 프로세스 3개의 스케줄링 동작을 실습 연습 3의 라운드 로빈과 비교하라고 한다.

6주차 실습 연습 3에서 라운드 로빈 아래의 `spin` 프로세스 세 개는 차례로 한 틱씩 받아 출력을 거의 고르게 번갈아 냈다. 여기에 대응하는 실험은 `priority_test`의 테스트 3이다. CPU를 쓰는 자식 세 개를 만들어 `setpriority`로 우선순위 1, 10, 19를 주므로, 우선순위를 준 `spin` 세 개와 작업 부하의 구조가 같다. 새 스케줄러에서 HIGH는 자신의 우선순위 값을 1로 바꾼 뒤로 엄격하게 가장 좋은 후보가 되어 끝날 때까지 CPU를 독차지하고, 그다음 MED가 끝까지 실행되며, 마지막으로 LOW가 실행된다.

이 순서가 출력의 `finished` 줄에서 보이려면 두 조건이 필요하다. 작업이 타임 퀀텀의 경계(`clockintr`가 정하는 약 100 ms)를 넘을 만큼 길어야 하고, 동점 처리가 돌아가야 한다. 배포된 `burn(5000000)`(5 × 10⁶번 반복)을 `CPUS=1`에서 실행하면 각 자식은 퀀텀 하나 안에 끝나므로, 순서는 단순히 만들어진 순서인 LOW, MED, HIGH가 된다. 7절의 출력이 이 경우이다. 반복 횟수를 5 × 10⁷로 늘리면 선점이 드러나 기대한 HIGH, MED, LOW 순서가 나타난다. 테스트 프로그램의 사본에서 반복 횟수만 바꾸어 같은 커널로 실행하면 다음과 같다.

```text
--- Test 3: High-priority process runs first ---
[HIGH prio=1] finished
[MED  prio=10] finished
[LOW  prio=19] finished
Test 3 PASSED
```

라운드 로빈과의 차이는 질적이다. 라운드 로빈은 끝없이 번갈아 실행하지만, 우선순위 스케줄링은 우선순위의 등급에 따라 실행을 직렬화한다.

> **참고:** 작업이 퀀텀 하나 안에 끝나면 스케줄러가 다시 고를 기회가 오지 않는다는 점은 라운드 로빈의 동작에서도 같다. 스케줄링 순서는 정책뿐 아니라 선점이 일어나는 시점, 곧 타임 퀀텀과 작업 길이의 관계로도 결정된다. 기대한 순서가 작업을 늘린 뒤에야 나타난 이 관찰은 그 관계를 보여 준다.

### 8.3 우선순위로 SJF 근사하기

질문 3은 평균 대기 시간에서 최적인 SJF를 우선순위 스케줄링으로 근사하는 방법과 필요한 정보를 묻는다.

SJF는 이미 우선순위가 예측한 다음 CPU 버스트(의 역수)인 우선순위 스케줄러이다. 이를 근사하려면 커널이 프로세스마다 다음 버스트 길이의 추정값을 유지하고, 스케줄링을 결정하기 전마다 그 값을 0\~20의 우선순위 척도로 옮긴다. 추정값은 관찰한 이력에서 얻으며, 보통 다음 지수 평균을 쓴다. 여기서 $t_n$은 가장 최근에 측정한 CPU 버스트, $\tau_n$은 그 이전까지의 예측값이다.

$$
\tau_{n+1} = \alpha t_n + (1 - \alpha) \tau_n, \qquad 0 \le \alpha \le 1
$$

측정하려면 문맥을 교환할 때마다 프로세스가 CPU를 양보하거나 블록되기 전까지 얼마나 실행했는지 기록해야 하며, I/O를 기다린 시간이 아니라 CPU 버스트만 센다. 따라서 필요한 정보는 `priority` 필드 옆 `struct proc` 안에 저장하는 프로세스별 버스트 이력이다. 다만 순수한 SJF는 깨지기 쉽다. 추정값이 실제를 따라간다는 가정에 기대고, 버스트가 매우 짧은 I/O 중심 프로세스들이 CPU를 독차지할 수 있다. 그래서 실제 시스템은 SJF를 단독으로 쓰지 않고, SJF와 비슷한 편향을 노화와 다단계 피드백 큐와 함께 쓴다.

---

<br>

## 요약

| 개념 | 핵심 요약 |
|:-----|:-----|
| 우선순위 상태 | `allocproc()`이 10으로 초기화하고 `freeproc()`이 0으로 되돌리며 `kfork()`가 자식에게 복사하고, `struct cpu`의 `sched_last`가 마지막으로 디스패치한 첨자를 기억한다. |
| 시스템 콜 | 처리 함수는 `argint()`로 인자를 읽어 `proc.c`의 `ksetpriority()`와 `kgetpriority()`에 맡기고, 이 함수들은 프로세스마다 락을 잡고 PID를 찾으며 실패하면 -1을 반환한다. |
| 선택과 락 | 한 번 훑으며 우선순위가 엄격하게 낮은 RUNNABLE 항목을 만날 때마다 이전 후보의 락을 풀고 새 후보의 락을 이어 잡으며, 이긴 프로세스의 락을 잡은 채로 `swtch()`한다. |
| 동점의 라운드 로빈 | 훑기의 시작을 `(sched_last + 1) % NPROC`로 돌려, 같은 우선순위의 후보 가운데 먼저 방문하는 쪽이 매번 바뀌게 한다. |
| 테스트 결과 | 세 테스트가 모두 통과하지만, 배포된 반복 횟수에서는 각 자식이 퀀텀 하나 안에 끝나 테스트 3의 순서가 만들어진 순서가 되고, 반복 횟수를 10배로 늘리면 우선순위 순서가 나타난다. |
| 기아와 노화 | 엄격한 우선순위는 낮은 프로세스를 굶길 수 있으며, 속성 때문에 생긴다는 점에서 FCFS의 호위 효과와 다르고, 노화로 해결한다. |
| SJF 근사 | 지수 평균으로 다음 버스트를 예측하여 우선순위로 옮기며, 문맥 교환마다 측정한 CPU 버스트의 이력이 필요하다. |

---

<br>

## 점검 문제

1. **동점의 고착:** 훑기를 언제나 `proc[0]`부터 시작하고 `p->priority < best->priority`로 비교하면, 우선순위 10인 프로세스 세 개가 첨자 3, 4, 5에 있을 때 어떤 일이 생기는지 쓰라.

   <details>
   <summary>정답</summary>

   첨자 3의 프로세스가 먼저 방문되어 `best`가 되고, 4와 5는 우선순위가 엄격하게 낮지 않으므로 바꾸지 못한다. 그 프로세스가 RUNNABLE인 동안에는 매번 첨자 3만 선택되어 4와 5가 굶는다.

   </details>

2. **훑기의 시작:** `NPROC = 64`이고 마지막으로 디스패치한 프로세스가 `proc[63]`일 때, 다음 훑기가 방문하는 첫 첨자와 마지막 첨자를 쓰라.

   <details>
   <summary>정답</summary>

   `c->sched_last = 63`이므로 첫 첨자는 `(63 + 1 + 0) % 64 = 0`이고, 마지막(i = 63)은 `(63 + 1 + 63) % 64 = 63`이다. 방금 실행한 프로세스는 마지막에 방문되므로, 동점이면 다른 프로세스가 먼저 뽑힌다.

   </details>

3. **지수 평균:** $\alpha = 0.5$, 처음 예측값 $\tau_0 = 10$ ms에서 실제 버스트가 6 ms, 4 ms로 측정되었을 때 $\tau_2$를 구하라.

   <details>
   <summary>정답</summary>

   $\tau_1 = 0.5 \times 6 + 0.5 \times 10 = 8$ ms이고, $\tau_2 = 0.5 \times 4 + 0.5 \times 8 = 6$ ms이다.

   </details>

4. **테스트 3의 순서:** 배포된 테스트 3이 `CPUS=1`에서 LOW, MED, HIGH 순서로 끝나도 스케줄러가 틀렸다고 할 수 없는 이유를 쓰라.

   <details>
   <summary>정답</summary>

   각 자식의 반복 횟수 5 × 10⁶은 타임 퀀텀 하나 안에 끝날 만큼 짧다. 그래서 먼저 실행된 자식이 우선순위를 바꾸고 작업을 마칠 때까지 타이머 인터럽트로 선점될 기회가 없고, 스케줄러가 다른 자식을 고를 시점은 그 자식이 끝난 뒤에야 온다. 반복 횟수를 5 × 10⁷로 늘리면 퀀텀마다 다시 고르게 되어 HIGH, MED, LOW 순서가 나타나므로, 우선순위 선택은 올바르게 동작한다.

   </details>
