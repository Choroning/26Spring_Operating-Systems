# HW5 과제 안내문 — xv6 우선순위 스케줄러

[레포지토리 안내](../../../README.ko.md) | [English](Assignment.md) | [한국어](Assignment.ko.md)

> **최종 수정일:** 2026-11-16

운영체제 5차 과제이다. 6주차(CPU 스케줄링 1)의 스케줄링 알고리즘과 실습에서 분석한 xv6 스케줄러를 바탕으로 xv6의 스케줄러를 바꾼다. 배포 파일인 테스트 프로그램 `priority_test.c`는 이 폴더에 원본 그대로 있다.

- **제출 기한:** 다음 수업 전
- **기반:** 6주차 이론(CPU 스케줄링)과 실습(xv6의 문맥 교환)
- **난이도:** ★★★☆☆
- **환경:** xv6-riscv (QEMU와 RISC-V 툴체인 필요, 2주차 설치 안내 `week02/2_lab/setup_xv6_env.md` 참고)

---

<br>

## 개요

이 과제에서는 xv6의 스케줄러를 고쳐 **우선순위 기반 스케줄링**을 구현한다.

현재 xv6는 `proc[]` 배열을 선형으로 훑어 처음 찾은 RUNNABLE 프로세스를 실행하는 단순한 **라운드 로빈** 스케줄러를 쓴다(실습 연습 2, 3에서 관찰한 대로이다). 이것을 스케줄러가 항상 **우선순위가 가장 높은** RUNNABLE 프로세스를 고르도록 바꾼다.

또 사용자 프로그램이 실행 중에 프로세스의 우선순위를 바꾸고 조회할 수 있도록 시스템 콜 두 개, `setpriority`와 `getpriority`를 더한다.

---

<br>

## 배경

### 이론에서: 스케줄링 알고리즘

6주차 강의에서는 여러 스케줄링 알고리즘을 배웠다.

- **FCFS**: 가장 먼저 도착한 프로세스를 고른다. 단순하지만 **호위 효과(convoy effect)** 가 생긴다
- **SJF**: 다음 CPU 버스트가 가장 짧은 프로세스를 고른다. 평균 대기 시간에서 최적이지만 버스트 길이를 미리 알아야 한다
- **RR**: 각 프로세스에 고정된 타임 퀀텀을 돌아가며 준다. 공정하지만 반환 시간이 길다

**SJF는 사실 우선순위 스케줄링의 특수한 경우**로, 우선순위가 예측한 CPU 버스트의 역수이다. 일반적인 생각은 각 프로세스에 숫자로 된 **우선순위**를 주고, 스케줄러가 항상 우선순위가 가장 높은 RUNNABLE 프로세스를 실행하는 것이다.

### 실습에서: xv6 스케줄러의 내부

실습에서는 xv6의 스케줄러를 분석했다.

- **연습 1**: `swtch.S`가 피호출자 저장 레지스터를 저장하고 복원하여 문맥을 전환한다
- **연습 2**: `kernel/proc.c`의 `scheduler()`가 `proc[]`를 선형으로 훑어 RUNNABLE 프로세스를 찾는다
- **연습 3**: `spin` 프로세스 여러 개로 라운드 로빈의 번갈아 실행을 관찰했다

추적한 핵심 코드는 다음과 같다.

```c
// 현재 스케줄러: 단순한 라운드 로빈
for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->state == RUNNABLE) {
        p->state = RUNNING;
        c->proc = p;
        swtch(&c->context, &p->context);
        c->proc = 0;
    }
    release(&p->lock);
}
```

과제는 이것을 우선순위에 따른 선택으로 바꾸는 것이다.

---

<br>

## 기능 명세

### 우선순위 규칙

| 속성 | 값 |
|----------|-------|
| 범위 | 0(가장 높음)부터 20(가장 낮음)까지 |
| 기본값 | 10 |
| 동점 처리 | 우선순위가 같은 프로세스끼리는 라운드 로빈 |

*표 1. 우선순위 규칙*

### 시스템 콜

#### `setpriority(int pid, int priority)`

프로세스 `pid`의 우선순위를 `priority`로 정한다.

- 성공하면 `0`을 반환한다
- `pid`가 없거나 `priority`가 범위(0\~20)를 벗어나면 `-1`을 반환한다
- 프로세스는 자신의 우선순위나 자식의 우선순위를 정할 수 있다

#### `getpriority(int pid)`

프로세스 `pid`의 우선순위를 반환한다.

- 성공하면 우선순위 값(0\~20)을 반환한다
- `pid`가 없으면 `-1`을 반환한다

### 스케줄러의 동작

스케줄러는 처음 찾은 RUNNABLE 프로세스를 실행하는 대신 다음과 같이 해야 한다.

1. **모든** RUNNABLE 프로세스를 훑는다
2. **우선순위 숫자가 가장 작은**(= 우선순위가 가장 높은) 프로세스를 고른다
3. 우선순위가 같은 프로세스가 여럿이면 그들 사이에서 **라운드 로빈**을 쓴다

---

<br>

## 단계별 구현 안내

### 수정할 파일 목록

| 파일 | 수정 내용 |
|------|-------------|
| `kernel/proc.h` | `struct proc`에 `priority` 필드 추가 |
| `kernel/proc.c` | `allocproc()`에서 우선순위 초기화, `freeproc()`에서 재설정, `kfork()`에서 복사, `scheduler()` 수정 |
| `kernel/syscall.h` | `SYS_setpriority`(22)와 `SYS_getpriority`(23) 추가 |
| `kernel/sysproc.c` | `sys_setpriority()`와 `sys_getpriority()` 구현 |
| `kernel/syscall.c` | syscalls 테이블에 추가 |
| `user/usys.pl` | 두 시스템 콜의 항목 추가 |
| `user/user.h` | 함수 선언 추가 |
| `Makefile` | 테스트 프로그램을 UPROGS에 추가 |

*표 2. 수정할 파일과 수정 내용*

### 1단계: `struct proc`에 우선순위 필드 추가

`kernel/proc.h`에 `priority` 필드를 추가한다.

```c
struct proc {
  // ... 기존 필드 ...
  char name[16];               // Process name (debugging)
  int priority;                // 스케줄링 우선순위 (0=가장 높음, 20=가장 낮음)
};
```

### 2단계: 우선순위의 초기화, 재설정, 상속

**(a)** `kernel/proc.c`의 `allocproc()`에서 기본 우선순위를 정한다.

```c
// 다른 초기화(예: pid 설정) 뒤에
// TODO: 기본 우선순위를 10으로 정한다
```

**(b)** `kernel/proc.c`의 `freeproc()`에서 우선순위를 재설정한다.

```c
// TODO: 우선순위를 0으로 재설정한다
```

**(c)** `kernel/proc.c`의 `kfork()`에서 부모의 우선순위를 자식에게 복사한다.

```c
// 다음 줄 뒤에: safestrcpy(np->name, p->name, sizeof(p->name));
// TODO: 부모의 우선순위를 자식에게 복사한다
```

### 3단계: 시스템 콜 번호 등록

`kernel/syscall.h`에 추가한다.

```c
#define SYS_setpriority 22
#define SYS_getpriority 23
```

### 4단계: 사용자 공간 인터페이스

`user/usys.pl`에 항목을 추가한다.

```perl
entry("setpriority");
entry("getpriority");
```

`user/user.h`에 선언을 추가한다.

```c
int setpriority(int, int);
int getpriority(int);
```

### 5단계: 시스템 콜 구현

`kernel/sysproc.c`에 두 함수를 구현한다.

#### `sys_setpriority()`

```c
uint64
sys_setpriority(void)
{
  // TODO:
  // 1. 인자를 읽는다: pid (argint 0), priority (argint 1)
  // 2. priority가 [0, 20] 범위인지 검사한다
  // 3. proc[]에서 주어진 pid의 프로세스를 찾는다
  // 4. 그 priority 필드를 정한다
  // 5. 성공하면 0, 실패하면 -1을 반환한다
}
```

**힌트:** 프로세스를 찾으려면 `proc[]` 배열(`defs.h`에 `extern`으로 선언됨)을 돌아야 한다. 접근하기 전에 `acquire(&p->lock)`을, 접근한 뒤에 `release(&p->lock)`을 잊지 않는다.

#### `sys_getpriority()`

```c
uint64
sys_getpriority(void)
{
  // TODO:
  // 1. 인자를 읽는다: pid (argint 0)
  // 2. proc[]에서 주어진 pid의 프로세스를 찾는다
  // 3. 그 우선순위를 반환하고, 없으면 -1을 반환한다
}
```

### 6단계: 시스템 콜 분배 테이블 수정

`kernel/syscall.c`에서 다음을 한다.

**(a)** 함수 원형을 추가한다.

```c
extern uint64 sys_setpriority(void);
extern uint64 sys_getpriority(void);
```

**(b)** `syscalls[]` 배열에 항목을 추가한다.

```c
[SYS_setpriority] sys_setpriority,
[SYS_getpriority] sys_getpriority,
```

### 7단계: 스케줄러 수정 (핵심 과제)

과제의 중심이다. `kernel/proc.c`의 `scheduler()`를 고친다.

**현재 동작** (라운드 로빈): 처음 찾은 RUNNABLE 프로세스를 실행한다.

**새 동작** (우선순위): 모든 프로세스를 훑어 우선순위 숫자가 가장 작은 것을 고른다.

```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();

  c->proc = 0;
  for(;;){
    intr_on();
    intr_off();

    // TODO: 기존의 라운드 로빈 루프를 우선순위에 따른 선택으로 바꾼다
    //
    // 알고리즘:
    //   1. proc[]의 모든 항목을 훑어 우선순위 숫자가 가장 작은
    //      (우선순위가 가장 높은) RUNNABLE 프로세스를 찾는다.
    //   2. 찾으면 상태를 RUNNING으로 바꾸고 swtch로 전환한다.
    //   3. RUNNABLE 프로세스가 없으면 wfi를 실행한다.
    //
    // 중요:
    //   - p->state를 검사하기 전에 p->lock을 잡고,
    //     p를 다 쓴 뒤에 풀어야 한다.
    //   - 동점(같은 우선순위)일 때는 라운드 로빈이면 된다
    //     (선형으로 훑으면 자연히 그렇게 된다).
    //
    // 힌트: 두 번 훑는 방법:
    //   1차: RUNNABLE 프로세스 가운데 가장 작은 우선순위 값을 찾는다
    //   2차: 그 우선순위 값을 가진 첫 RUNNABLE 프로세스를 실행한다
    //
    //   또는: 한 번 훑으며 "지금까지 가장 좋은" 후보를 기억한다.

    /* ====== YOUR CODE HERE ====== */


    /* ====== END YOUR CODE ====== */
  }
}
```

**락에 관한 힌트:** 한 번 훑는 방법이 더 단순하다. 지금까지 찾은 가장 좋은 후보를 가리키는 포인터를 둔다. 더 좋은 후보를 찾으면 이전 후보의 락을 풀고 새 후보의 락을 잡는다.

```
best = NULL
for each p in proc[]:
    acquire(&p->lock)
    if p is RUNNABLE and (best == NULL or p->priority < best->priority):
        if best != NULL: release(&best->lock)
        best = p
    else:
        release(&p->lock)

if best != NULL:
    // best를 디스패치한다 (이미 락이 잡혀 있다)
    best->state = RUNNING
    c->proc = best
    swtch(...)
    c->proc = 0
    release(&best->lock)
```

---

<br>

## 테스트

### 테스트 프로그램

`priority_test.c`를 `xv6-riscv/user/`에 복사하고 `Makefile`의 `UPROGS`에 `$U/_priority_test\`를 추가한다.

```bash
make clean && make qemu

# xv6 셸에서:
$ priority_test
```

### 예상 출력

```
=== Priority Scheduler Test ===

--- Test 1: setpriority/getpriority ---
PID 4: default priority = 10
PID 4: after setpriority(4, 5), priority = 5
setpriority with invalid priority (-1): returned -1 (OK)
setpriority with invalid priority (21): returned -1 (OK)
Test 1 PASSED

--- Test 2: Priority inheritance via fork ---
Parent priority = 3
Child priority = 3
Test 2 PASSED

--- Test 3: High-priority process runs first ---
[HIGH prio=1] finished
[MED  prio=10] finished
[LOW  prio=19] finished
Test 3 PASSED

All tests passed!
```

테스트 3에서 **눈여겨볼 점:** 우선순위가 높은 프로세스(우선순위 1)가 우선순위가 중간인 프로세스(우선순위 10)보다 먼저 모든 작업을 마치고, 중간인 프로세스는 우선순위가 낮은 프로세스(우선순위 19)보다 먼저 마쳐야 한다. 이는 모든 프로세스가 고르게 번갈아 실행되던 실습의 라운드 로빈 동작과 다르다.

---

<br>

## 제출 방법

다음 중 한 가지 방법으로 제출한다.

**방법 1: 패치 파일** (권장)
```bash
cd xv6-riscv
git diff > priority.patch
```

**방법 2: 수정한 파일**

다음 파일을 제출한다.
- `kernel/proc.h`
- `kernel/proc.c`
- `kernel/syscall.h`
- `kernel/syscall.c`
- `kernel/sysproc.c`
- `user/usys.pl`
- `user/user.h`

---

<br>

## 채점 기준

| 항목 | 비중 |
|------|--------|
| `priority` 필드를 추가하고 올바르게 초기화, 재설정, 상속 | 15% |
| `setpriority` 시스템 콜이 올바르게 동작 (검증 포함) | 15% |
| `getpriority` 시스템 콜이 올바르게 동작 | 10% |
| `scheduler()`가 우선순위가 가장 높은 프로세스를 선택 | 40% |
| 테스트 프로그램이 모든 테스트를 통과 | 10% |
| 코드 스타일과 주석 | 10% |

*표 3. 채점 기준*

---

<br>

## 이론과 실습의 대응

| 과제의 작업 | 이론의 개념 | 실습의 연습 |
|---|---|---|
| `scheduler()` 수정 | 스케줄링 알고리즘 (RR 너머) | 연습 2: `scheduler()` 추적, 연습 3: RR 관찰 |
| `struct proc`에 `priority` 추가 | 스케줄링 결정을 위한 프로세스 속성 | 연습 1: `struct context` 분석, 연습 3: `struct proc` 학습 |
| `setpriority`/`getpriority` 시스템 콜 | 해당 없음 (3주차의 시스템 콜 메커니즘) | 3주차 실습: 시스템 콜 경로 추적 |
| 스케줄링 순서 관찰 | 스케줄링 기준 (대기 시간, 응답 시간) | 연습 3: 번갈아 실행 관찰 |
| 높은 우선순위의 기아 | 호위 효과 (FCFS), SJF 최적성의 트레이드오프 | 연습 4: sleep/wakeup 상태 전이 |

*표 4. 과제의 작업과 이론, 실습의 대응*

---

<br>

## 질문 (보고서에 포함)

1. 이 우선순위 스케줄러에서 우선순위가 높은 프로세스가 계속 도착하면 우선순위가 낮은 프로세스는 어떻게 되는가? 이는 이론에서 배운 FCFS의 **호위 효과**와 어떤 관계가 있는가?

2. 우선순위가 1, 10, 19인 `spin` 프로세스 3개를 실행하라(래퍼에서 `setpriority` 사용). 그 스케줄링 동작을 실습 연습 3에서 관찰한 라운드 로빈 동작과 비교하라. 무엇이 다른가?

3. SJF는 평균 대기 시간을 최소화하는 데 최적이라고 알려져 있다. 우선순위 스케줄링으로 SJF를 근사하려면 어떻게 하겠는가? 어떤 정보가 필요한가?

---

<br>

## 참고 자료

- Silberschatz Ch 5 (5.1\~5.3절): CPU Scheduling Algorithms
- xv6 교재 Chapter 7: Scheduling
- 3주차 과제: xv6에 시스템 콜 추가
- 6주차 실습: 문맥 교환과 스케줄러 추적

---

<br>

문제를 풀어 본 뒤 [과제 해설](../Assignment-Explanation.ko.md)에서 정답과 풀이를, [코드 총정리](../Code-Walkthrough.ko.md)에서 작성한 코드 전체를 볼 수 있다.
