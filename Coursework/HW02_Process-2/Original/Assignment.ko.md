# HW2 과제 안내문 — trace 시스템 콜 추가

[레포지토리 안내](../../../README.ko.md) | [English](Assignment.md) | [한국어](Assignment.ko.md)

> **최종 수정일:** 2026-11-16

운영체제 2차 과제이다. 3주차(프로세스 2)의 실습에서 따라간 xv6의 시스템 콜 경로를 바탕으로, xv6에 새 시스템 콜을 더한다. 배포 파일인 테스트 프로그램 `trace_test.c`와 TODO 주석이 들어 있는 패치 `trace.patch`는 이 폴더에 원본 그대로 있다.

- **제출 기한:** 다음 수업 전
- **기반:** MIT 6.1810 Lab: System Calls
- **난이도:** 필수(trace) + 보너스(sysinfo)
- **환경:** xv6-riscv (QEMU와 RISC-V 툴체인 필요, 2주차 설치 안내 `week02/2_lab/setup_xv6_env.md` 참고)

---

<br>

## 개요

이 과제에서는 xv6에 새 시스템 콜 `trace(int mask)`를 추가한다.
그 과정에서 시스템 콜이 어떻게 등록되고, 사용자 프로그램에서 커널까지 어떤 경로를 거치는지 직접 경험한다.

---

<br>

## 과제 1: `trace` 시스템 콜 (필수)

### 기능 명세

`trace(int mask)` 시스템 콜은 호출한 프로세스의 시스템 콜 추적을 켠다.

- **mask**: 추적할 시스템 콜의 비트마스크이다. 시스템 콜 번호 `n`을 추적하려면 비트 `(1 << n)`을 켠다.
- 추적하는 시스템 콜이 호출될 때마다 커널은 다음 형식의 줄을 출력한다.

  ```
  <pid>: syscall <syscall_name> -> <return_value>
  ```

  예:
  ```
  3: syscall fork -> 4
  4: syscall write -> 1
  ```

- `fork()`로 만든 자식 프로세스는 부모의 추적 마스크를 **상속**한다.
- `trace(0)`을 호출하면 추적이 꺼진다.

### 사용 예

```c
// trace(1 << SYS_fork)를 호출하면 fork 시스템 콜을 추적한다
// fork가 호출될 때마다 로그가 출력된다

$ trace 1 fork    // mask = (1 << SYS_fork) = 2
3: syscall fork -> 4

$ trace 5 read write  // mask = (1<<SYS_read)|(1<<SYS_write) = 0x10020
// read나 write가 호출될 때마다 로그가 출력된다
```

---

<br>

## 단계별 구현 안내

### 수정할 파일 목록

| 파일 | 수정 내용 |
|------|-------------|
| `kernel/syscall.h` | `SYS_trace` 번호(22) 추가 |
| `user/usys.pl` | `trace` 항목 추가 |
| `user/user.h` | `trace()` 함수 선언 추가 |
| `kernel/proc.h` | `struct proc`에 `trace_mask` 필드 추가 |
| `kernel/sysproc.c` | `sys_trace()` 함수 구현 |
| `kernel/syscall.c` | syscalls 테이블에 추가, `syscall()`에 추적 로직 추가 |
| `kernel/proc.c` | `kfork()`에서 `trace_mask` 복사 |
| `Makefile` | 테스트 프로그램을 UPROGS에 추가 (선택) |

*표 1. 수정할 파일과 수정 내용*

### 1단계: 시스템 콜 번호 등록

`kernel/syscall.h`에 새 시스템 콜 번호를 추가한다.

```c
// kernel/syscall.h
// 기존의 마지막 줄:
#define SYS_close  21

// 추가:
#define SYS_trace  22
```

### 2단계: 사용자 공간 인터페이스

**`user/usys.pl`** 파일의 끝에 항목을 추가한다.

```perl
entry("trace");
```

그러면 빌드할 때 `usys.S`에 다음 어셈블리가 생성된다.

```asm
.global trace
trace:
 li a7, SYS_trace
 ecall
 ret
```

**`user/user.h`** 에 함수 선언을 추가한다.

```c
// 시스템 콜 선언 부분에 추가
int trace(int);
```

### 3단계: 프로세스 구조체에 필드 추가

`kernel/proc.h`의 `struct proc`에 추적 마스크를 저장할 필드를 추가한다.

```c
struct proc {
  // ... 기존 필드 ...
  char name[16];               // Process name (debugging)
  int trace_mask;              // 추적할 시스템 콜 마스크 (추가)
};
```

### 4단계: `sys_trace()` 구현

`kernel/sysproc.c`에 `sys_trace()` 함수를 구현한다.

이 함수는 다음을 해야 한다.
1. `argint()`로 첫 번째 인자(mask)를 읽는다
2. 그 값을 현재 프로세스의 `trace_mask` 필드에 저장한다

```c
uint64
sys_trace(void)
{
  // TODO:
  // 1. argint()로 첫 번째 인자(mask)를 읽는다
  // 2. myproc()->trace_mask에 저장한다
  // 3. 성공하면 0을 반환한다
}
```

**힌트:** `sys_kill()` 함수를 참고한다. `argint()`로 인자를 읽는 방식이 같다.

```c
// 참고: kernel/sysproc.c의 sys_kill()
uint64
sys_kill(void)
{
  int pid;
  argint(0, &pid);
  return kkill(pid);
}
```

### 5단계: syscall 테이블과 분배 로직 수정

**`kernel/syscall.c`** 에서 세 가지를 수정한다.

**(a)** 함수 원형을 추가한다.

```c
extern uint64 sys_trace(void);
```

**(b)** `syscalls[]` 배열에 항목을 추가한다.

```c
static uint64 (*syscalls[])(void) = {
  // ... 기존 항목 ...
  [SYS_close]   sys_close,
  [SYS_trace]   sys_trace,   // 추가
};
```

**(c)** 시스템 콜 이름 배열을 추가하고 `syscall()` 함수에 추적 로직을 추가한다.

```c
// 시스템 콜 이름 문자열 배열 (추적 출력용)
static char *syscall_names[] = {
  [SYS_fork]    "fork",
  [SYS_exit]    "exit",
  [SYS_wait]    "wait",
  [SYS_pipe]    "pipe",
  [SYS_read]    "read",
  [SYS_kill]    "kill",
  [SYS_exec]    "exec",
  [SYS_fstat]   "fstat",
  [SYS_chdir]   "chdir",
  [SYS_dup]     "dup",
  [SYS_getpid]  "getpid",
  [SYS_sbrk]    "sbrk",
  [SYS_pause]   "pause",
  [SYS_uptime]  "uptime",
  [SYS_open]    "open",
  [SYS_write]   "write",
  [SYS_mknod]   "mknod",
  [SYS_unlink]  "unlink",
  [SYS_link]    "link",
  [SYS_mkdir]   "mkdir",
  [SYS_close]   "close",
  [SYS_trace]   "trace",
};

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num]();

    // TODO: 추적 로직 추가
    // p->trace_mask에서 num번 비트가 켜져 있으면
    // printf로 "<pid>: syscall <name> -> <return_value>"를 출력한다
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
```

**힌트:**
- 비트마스크 검사는 `(p->trace_mask & (1 << num))` 식을 쓴다.
- `p->trapframe->a0`은 `uint64` 형이므로 `printf`의 `%d`와 함께 쓸 때는 `(int)`로 캐스트한다.
  (`-Werror` 플래그 때문에 형 불일치 경고가 컴파일 오류로 처리된다.)

### 6단계: `fork()`에서 trace_mask 상속

`kernel/proc.c`의 `kfork()` 함수에 자식 프로세스가 부모의 `trace_mask`를 상속하도록 한 줄을 추가한다.

```c
int
kfork(void)
{
  // ...
  np->sz = p->sz;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // TODO: 부모의 trace_mask를 자식에게 복사한다
  // np->trace_mask = ???;

  // increment reference counts on open file descriptors.
  // ...
}
```

### 7단계: 프로세스를 초기화할 때 trace_mask 초기화

`kernel/proc.c`의 `freeproc()` 함수에서 `trace_mask`를 0으로 초기화해야 한다.

```c
static void
freeproc(struct proc *p)
{
  // ... 기존 코드 ...
  p->state = UNUSED;
  // TODO: p->trace_mask = 0;을 추가한다
}
```

---

<br>

## 테스트

### 테스트 프로그램

`trace_test.c`를 `user/` 디렉토리에 복사하고 `Makefile`의 `UPROGS`에 추가한 뒤 테스트를 실행한다.
(`trace_test.c`는 과제 배포 파일에 들어 있다.)

```bash
# xv6 빌드와 실행
make clean && make qemu

# xv6 셸에서 테스트
$ trace_test
```

### 예상 출력

```
=== Test 1: trace fork ===
<pid>: syscall fork -> <child_pid>
Test 1 PASSED

=== Test 2: trace read/write ===
(write와 read의 추적 출력이 나타남)
Test 2 PASSED

=== Test 3: trace multiple syscalls ===
(여러 시스템 콜의 추적 출력)
Test 3 PASSED

=== Test 4: trace inheritance ===
(자식 프로세스의 추적 출력도 나타남)
Test 4 PASSED

All tests passed!
```

### 수동 테스트

xv6 셸에서 직접 확인할 수도 있다.

```bash
# 간단한 프로그램을 작성해 테스트
$ trace_test
```

---

<br>

## 제출 방법

다음 중 한 가지 방법으로 제출한다.

**방법 1: 패치 파일**
```bash
cd xv6-riscv
git diff > trace.patch
```

**방법 2: 수정한 파일**

다음 파일을 제출한다.
- `kernel/syscall.h`
- `kernel/syscall.c`
- `kernel/sysproc.c`
- `kernel/proc.h`
- `kernel/proc.c`
- `user/usys.pl`
- `user/user.h`

---

<br>

## 과제 2: `sysinfo` 시스템 콜 (보너스, +10점)

### 기능 명세

`sysinfo(struct sysinfo *)` 시스템 콜을 추가한다. 이 시스템 콜은 현재 시스템 상태에 관한 정보를 돌려준다.

### 구현할 내용

1. `kernel/sysinfo.h` 파일을 만든다.

```c
struct sysinfo {
  uint64 freemem;   // 사용 가능한 메모리 (바이트)
  uint64 nproc;     // 활성 프로세스 수 (state != UNUSED)
};
```

2. `SYS_sysinfo`를 23번으로 등록한다

3. 필요한 보조 함수를 구현한다.
   - `kernel/kalloc.c`의 `freemem_count()`: free list를 순회하여 빈 페이지 수 * PGSIZE를 반환한다
   - `kernel/proc.c`의 `proc_count()`: `proc[]` 배열을 순회하여 UNUSED가 아닌 프로세스의 수를 반환한다

4. `kernel/sysproc.c`에 `sys_sysinfo()`를 구현한다.
   - `struct sysinfo` 데이터를 사용자 공간 포인터로 복사한다(`copyout()` 사용)

### 힌트

- 커널 데이터를 사용자 공간으로 복사할 때는 `copyout()`을 쓴다
- 사용자 포인터는 `argaddr()`로 받는다
- `sys_fstat()`의 구현 방식을 참고한다

---

<br>

## 채점 기준

| 항목 | 배점 |
|------|-------|
| `trace` 시스템 콜이 올바르게 등록됨 (컴파일 성공) | 20점 |
| `sys_trace()` 구현과 마스크 저장 | 20점 |
| `syscall()`이 올바른 형식으로 추적 출력 | 30점 |
| `fork()`에서 trace_mask 상속이 동작 | 20점 |
| 코드 스타일과 주석 | 10점 |
| (보너스) `sysinfo` 시스템 콜 | +10점 |

*표 2. 채점 기준*

---

<br>

## 참고 자료

- xv6 교재 Chapter 2: Operating System Organization
- xv6 교재 Chapter 4: Traps and System Calls
- [MIT 6.1810 Lab: System Calls](https://pdos.csail.mit.edu/6.828/2024/labs/syscall.html)
- 수업 실습에서 다룬 시스템 콜 경로 분석 (3주차 실습)

---

<br>

문제를 풀어 본 뒤 [과제 해설](../Assignment-Explanation.ko.md)에서 정답과 풀이를, [코드 총정리](../Code-Walkthrough.ko.md)에서 작성한 코드 전체를 볼 수 있다.
