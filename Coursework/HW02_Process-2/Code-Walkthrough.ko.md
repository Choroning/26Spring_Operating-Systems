# HW2 코드 총정리 — trace 시스템 콜 추가

[레포지토리 안내](../../README.ko.md) | [English](Code-Walkthrough.md) | [한국어](Code-Walkthrough.ko.md)

> **최종 수정일:** 2026-11-16

> **학습 목표:**
> 1. 통합 diff 형식의 패치에서 파일, 변경 위치, 더한 줄과 바꾸지 않은 줄을 읽을 수 있다
> 2. 패치의 각 변경이 배포된 패치에 이미 있던 것인지, 필수 과제와 보너스 과제 중 어디에 속하는지 구분할 수 있다
> 3. `syscall()`이 시스템 콜 번호로 처리 함수와 이름을 찾고 추적 줄을 출력하는 과정을 줄 단위로 설명할 수 있다
> 4. `freemem_count()`, `proc_count()`, `sys_sysinfo()`가 락과 `copyout()`을 쓰는 방식을 설명할 수 있다

---

## 목차

- [1. 파일 구성](#1-파일-구성)
- [2. 패치를 읽는 법](#2-패치를-읽는-법)
- [3. 빌드 설정과 원형](#3-빌드-설정과-원형)
- [4. kernel/kalloc.c](#4-kernelkallocc)
- [5. kernel/proc.c와 kernel/proc.h](#5-kernelprocc와-kernelproch)
- [6. kernel/syscall.c와 kernel/syscall.h](#6-kernelsyscallc와-kernelsyscallh)
- [7. kernel/sysinfo.h와 kernel/sysproc.c](#7-kernelsysinfoh와-kernelsysprocc)
- [8. 사용자 쪽 파일](#8-사용자-쪽-파일)
- [요약](#요약)
- [점검 문제](#점검-문제)

---

<br>

## 1. 파일 구성

> **풀어 보기:** 이 문서는 작성한 코드 전체와 줄별 설명을 담고 있다. 먼저 [과제 안내문](Original/Assignment.ko.md)의 문제를 직접 풀어 본 뒤 읽는다.

짝이 되는 문서 [HW2 과제 해설](Assignment-Explanation.ko.md)은 요구사항, 시스템 콜 경로, 구현 단계, 테스트 결과를 정리한다. 이 문서는 제출한 `Solution/trace.patch`(450줄)를 처음부터 끝까지 파일 순서대로 읽는다. 패치는 xv6 작업 트리에서 `git diff`로 만든 것이므로 파일이 경로의 알파벳 순서로 나온다.

| 패치 줄 | 파일 | 변경 | 배포된 패치와의 관계 |
|:-----|:-----|:-----|:-----|
| 1\~12 | `Makefile` | `UPROGS`에 `_trace_test` 추가 | 추가 (안내문의 선택 항목) |
| 13\~32 | `kernel/defs.h` | `freemem_count()`, `proc_count()` 원형 | 추가 (보너스) |
| 33\~58 | `kernel/kalloc.c` | `freemem_count()` | 추가 (보너스) |
| 59\~103 | `kernel/proc.c` | `freeproc()`의 초기화, `kfork()`의 복사, `proc_count()` | TODO 작성, `proc_count()`는 추가 (보너스) |
| 104\~113 | `kernel/proc.h` | `trace_mask` 필드 | 그대로 |
| 114\~174 | `kernel/syscall.c` | 원형, 테이블 항목, `syscall_names[]`, 추적 출력 | `trace` 원형과 항목은 그대로, 나머지는 작성, `sysinfo` 항목은 추가 |
| 175\~184 | `kernel/syscall.h` | `SYS_trace`, `SYS_sysinfo` | `SYS_trace`는 그대로, `SYS_sysinfo`는 추가 |
| 185\~194 | `kernel/sysinfo.h` | `struct sysinfo` (새 파일) | 추가 (보너스) |
| 195\~239 | `kernel/sysproc.c` | 헤더 포함, `sys_trace()`, `sys_sysinfo()` | `sys_trace()` 본문 작성, 나머지는 추가 (보너스) |
| 240\~426 | `user/trace_test.c` | 테스트 프로그램 (새 파일) | 배포된 `Original/trace_test.c`와 같음 |
| 427\~440 | `user/user.h` | `trace()`, `sysinfo()` 선언 | `trace()`는 그대로, `sysinfo()`는 추가 |
| 441\~450 | `user/usys.pl` | `trace`, `sysinfo` 항목 | `trace`는 그대로, `sysinfo`는 추가 |

*표 1. 패치의 파일별 구성과 배포된 패치와의 관계*

표의 "그대로"는 배포된 `Original/trace.patch`에 같은 줄이 이미 있었다는 뜻이고, "작성"은 배포된 패치의 TODO 주석 자리를 채웠다는 뜻이다.

---

<br>

## 2. 패치를 읽는 법

패치는 통합 diff(unified diff) 형식이다. `diff --git a/경로 b/경로` 줄이 파일 하나의 시작이고, `index` 줄은 바뀌기 전과 뒤의 내용 해시와 파일 모드, `---`와 `+++` 줄은 바뀌기 전과 뒤의 파일 이름이다. `@@ -169,6 +169,7 @@ freeproc(struct proc *p)` 같은 줄은 변경 단위(hunk)의 머리로, 원래 파일의 169번 줄부터 6줄이 새 파일의 169번 줄부터 7줄이 되었다는 뜻이며, 뒤의 글자는 그 위치를 둘러싼 함수 이름이다. 본문에서 `+`로 시작하는 줄은 더한 줄, 공백으로 시작하는 줄은 바뀌지 않은 앞뒤 문맥이다. 이 패치에는 지운 줄(`-`)이 없다.

`index` 줄의 해시 `22a5401`(proc.c), `076d965`(syscall.c) 등은 배포된 패치의 해시와 같으므로, 두 패치가 같은 xv6 커밋을 기준으로 만들어졌음을 알 수 있다.

---

<br>

## 3. 빌드 설정과 원형

```diff
diff --git a/Makefile b/Makefile
index b262c0a..2c89047 100755
--- a/Makefile
+++ b/Makefile
@@ -145,6 +145,7 @@ UPROGS=\
 	$U/_logstress\
 	$U/_forphan\
 	$U/_dorphan\
+	$U/_trace_test\
 
 fs.img: mkfs/mkfs README $(UPROGS)
 	mkfs/mkfs fs.img README $(UPROGS)
diff --git a/kernel/defs.h b/kernel/defs.h
index 122d9ca..2231b69 100755
--- a/kernel/defs.h
+++ b/kernel/defs.h
@@ -59,6 +59,7 @@ void            ireclaim(int);
 void*           kalloc(void);
 void            kfree(void *);
 void            kinit(void);
+uint64          freemem_count(void);
 
 // log.c
 void            initlog(int, struct superblock*);
@@ -101,6 +102,7 @@ void            yield(void);
 int             either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
 int             either_copyin(void *dst, int user_src, uint64 src, uint64 len);
 void            procdump(void);
+uint64          proc_count(void);
 
 // swtch.S
 void            swtch(struct context*, struct context*);
```

9번 줄은 `Makefile`의 `UPROGS`에 `$U/_trace_test`를 더한다. `UPROGS`에 있는 프로그램은 빌드할 때 파일 시스템 이미지 `fs.img`에 들어가므로, xv6 셸에서 `trace_test`로 실행할 수 있다. 안내문은 이 변경을 선택 항목으로 두었다.

21번 줄과 29번 줄은 보너스 과제의 두 보조 함수 원형을 `kernel/defs.h`의 `kalloc.c` 절과 `proc.c` 절에 각각 더한다. `defs.h`는 커널의 모든 `.c` 파일이 포함하는 공용 선언 헤더이므로, 여기에 원형을 두면 `sysproc.c`에서 두 함수를 부를 수 있다.

---

<br>

## 4. kernel/kalloc.c

```diff
diff --git a/kernel/kalloc.c b/kernel/kalloc.c
index 0699e7e..7bd93e3 100755
--- a/kernel/kalloc.c
+++ b/kernel/kalloc.c
@@ -80,3 +80,21 @@ kalloc(void)
     memset((char*)r, 5, PGSIZE); // fill with junk
   return (void*)r;
 }
+
+// Count the number of bytes of free memory
+uint64
+freemem_count(void)
+{
+  struct run *r;
+  uint64 count = 0;
+
+  acquire(&kmem.lock);
+  r = kmem.freelist;
+  while(r) {
+    count++;
+    r = r->next;
+  }
+  release(&kmem.lock);
+
+  return count * PGSIZE;
+}
```

`freemem_count()`는 파일 끝, `kalloc()` 바로 뒤에 들어간다. 46번 줄의 `struct run`은 빈 페이지의 첫 부분을 연결 리스트의 노드로 쓰는 `kalloc.c`의 구조체이다. 49번 줄에서 `kmem.lock`을 잡고, 50\~54번 줄이 `kmem.freelist`부터 `next`를 따라가며 노드를 센 뒤, 55번 줄에서 락을 푼다. 57번 줄은 페이지 수에 `PGSIZE`를 곱해 바이트 수로 돌려준다. 결과를 `uint64`로 둔 것은 `struct sysinfo`의 필드 형과 맞추기 위해서이다.

---

<br>

## 5. kernel/proc.c와 kernel/proc.h

```diff
diff --git a/kernel/proc.c b/kernel/proc.c
index 22a5401..9d62297 100755
--- a/kernel/proc.c
+++ b/kernel/proc.c
@@ -169,6 +169,7 @@ freeproc(struct proc *p)
   p->killed = 0;
   p->xstate = 0;
   p->state = UNUSED;
+  p->trace_mask = 0;
 }
 
 // Create a user page table for a given process, with no user memory,
@@ -282,6 +283,9 @@ kfork(void)
   // Cause fork to return 0 in the child.
   np->trapframe->a0 = 0;
 
+  // Copy trace_mask from parent to child
+  np->trace_mask = p->trace_mask;
+
   // increment reference counts on open file descriptors.
   for(i = 0; i < NOFILE; i++)
     if(p->ofile[i])
```

첫 번째 변경 단위는 `freeproc()`의 끝, `p->state = UNUSED;` 바로 뒤에 `p->trace_mask = 0;`(67번 줄)을 더한다. 배포된 패치의 TODO 7 자리이다. 두 번째 변경 단위는 `kfork()`에서 자식의 a0을 0으로 둔 뒤, 열린 파일의 참조 수를 늘리기 전에 마스크를 복사한다(75\~76번 줄). 75번 줄의 주석은 배포된 패치의 TODO 주석에서 `TODO:`를 뺀 것이다. 이 시점의 `p`는 `myproc()`, 곧 `fork()`를 부른 부모이고 `np`는 `allocproc()`이 마련한 자식이다.

```diff
@@ -659,6 +663,22 @@ either_copyin(void *dst, int user_src, uint64 src, uint64 len)
   }
 }
 
+// Count the number of active processes (state != UNUSED)
+uint64
+proc_count(void)
+{
+  struct proc *p;
+  uint64 count = 0;
+
+  for(p = proc; p < &proc[NPROC]; p++) {
+    acquire(&p->lock);
+    if(p->state != UNUSED)
+      count++;
+    release(&p->lock);
+  }
+  return count;
+}
+
 // Print a process listing to console.  For debugging.
 // Runs when user types ^P on console.
 // No lock to avoid wedging a stuck machine further.
diff --git a/kernel/proc.h b/kernel/proc.h
index d021857..60559ee 100755
--- a/kernel/proc.h
+++ b/kernel/proc.h
@@ -104,4 +104,5 @@ struct proc {
   struct file *ofile[NOFILE];  // Open files
   struct inode *cwd;           // Current directory
   char name[16];               // Process name (debugging)
+  int trace_mask;              // Trace syscall mask
 };
```

세 번째 변경 단위는 `either_copyin()` 뒤에 `proc_count()`를 더한다(85\~99번 줄). 92번 줄의 반복문은 전역 배열 `proc`의 모든 슬롯을 차례로 가리키고, 93\~96번 줄이 슬롯마다 락을 잡고 `state`를 읽은 뒤 바로 푼다. 한 번에 락 하나만 잡으므로 세는 동안 다른 슬롯의 상태가 바뀔 수 있어, 결과는 순회하던 순간들의 근사값이다. 사용 중인 프로세스 수를 보고하는 용도로는 이 정도로 충분하다.

`kernel/proc.h`의 변경(112번 줄)은 `struct proc`의 마지막 필드로 `int trace_mask`를 더하는 것으로, 배포된 패치와 같다.

---

<br>

## 6. kernel/syscall.c와 kernel/syscall.h

```diff
diff --git a/kernel/syscall.c b/kernel/syscall.c
index 076d965..fca98f1 100755
--- a/kernel/syscall.c
+++ b/kernel/syscall.c
@@ -101,6 +101,8 @@ extern uint64 sys_unlink(void);
 extern uint64 sys_link(void);
 extern uint64 sys_mkdir(void);
 extern uint64 sys_close(void);
+extern uint64 sys_trace(void);
+extern uint64 sys_sysinfo(void);
 
 // An array mapping syscall numbers from syscall.h
 // to the function that handles the system call.
@@ -126,6 +128,35 @@ static uint64 (*syscalls[])(void) = {
 [SYS_link]    sys_link,
 [SYS_mkdir]   sys_mkdir,
 [SYS_close]   sys_close,
+[SYS_trace]   sys_trace,
+[SYS_sysinfo] sys_sysinfo,
+};
+
+// Syscall name string array (for trace output)
+static char *syscall_names[] = {
+[SYS_fork]    "fork",
+[SYS_exit]    "exit",
+[SYS_wait]    "wait",
+[SYS_pipe]    "pipe",
+[SYS_read]    "read",
+[SYS_kill]    "kill",
+[SYS_exec]    "exec",
+[SYS_fstat]   "fstat",
+[SYS_chdir]   "chdir",
+[SYS_dup]     "dup",
+[SYS_getpid]  "getpid",
+[SYS_sbrk]    "sbrk",
+[SYS_pause]   "pause",
+[SYS_uptime]  "uptime",
+[SYS_open]    "open",
+[SYS_write]   "write",
+[SYS_mknod]   "mknod",
+[SYS_unlink]  "unlink",
+[SYS_link]    "link",
+[SYS_mkdir]   "mkdir",
+[SYS_close]   "close",
+[SYS_trace]   "trace",
+[SYS_sysinfo] "sysinfo",
 };
```

122\~123번 줄은 두 처리 함수의 원형이다. 처리 함수는 `sysproc.c`에 정의되어 있으므로 `extern`으로 선언한다. 131\~132번 줄은 `syscalls[]`에 두 항목을 더하고, 133번 줄의 `};`로 배열을 닫는다. 135\~160번 줄은 배포된 패치의 TODO 자리에 쓴 `syscall_names[]`로, 기존 21개 시스템 콜과 새 `trace`, `sysinfo`의 이름을 번호 순서대로 담는다. 160번 줄의 `};`는 원래 `syscalls[]`를 닫던 줄이 이제 이 배열을 닫게 된 것이다. 이름은 문자열 상수이므로 커널의 읽기 전용 데이터에 놓인다.

```diff
 
 void
@@ -139,6 +170,11 @@ syscall(void)
     // Use num to lookup the system call function for num, call it,
     // and store its return value in p->trapframe->a0
     p->trapframe->a0 = syscalls[num]();
+
+    // Trace: if the bit for this syscall is set, print trace info
+    if(p->trace_mask & (1 << num)) {
+      printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], (int)p->trapframe->a0);
+    }
   } else {
     printf("%d %s: unknown sys call %d\n",
             p->pid, p->name, num);
```

`syscall()`은 트랩 프레임의 a7에서 번호를 읽고, 번호가 1 이상이며 배열 범위 안에 있고 처리 함수가 등록되어 있을 때만 호출한다. 패치는 반환값을 a0에 저장하는 줄 바로 뒤에 추적 검사를 넣는다(167\~171번 줄). 169번 줄의 조건은 마스크에서 `num`번 비트만 남긴 값이 0이 아닌지를 본다. 170번 줄은 `p->pid`, 이름 배열의 문자열, `(int)`로 캐스트한 반환값으로 안내문의 형식을 그대로 출력한다. 커널의 `printf()`는 콘솔에 바로 쓰므로, 사용자 프로그램의 출력과 섞여 보일 수 있다.

```diff
diff --git a/kernel/syscall.h b/kernel/syscall.h
index 3dd926d..65d8104 100755
--- a/kernel/syscall.h
+++ b/kernel/syscall.h
@@ -20,3 +20,5 @@
 #define SYS_link   19
 #define SYS_mkdir  20
 #define SYS_close  21
+#define SYS_trace  22
+#define SYS_sysinfo 23
```

`kernel/syscall.h`는 `SYS_trace`를 22번으로, `SYS_sysinfo`를 23번으로 정의한다. 이 헤더는 커널과 사용자 프로그램이 함께 포함하므로, 두 쪽이 같은 번호를 쓴다.

---

<br>

## 7. kernel/sysinfo.h와 kernel/sysproc.c

```diff
diff --git a/kernel/sysinfo.h b/kernel/sysinfo.h
new file mode 100644
index 0000000..d7eab3f
--- /dev/null
+++ b/kernel/sysinfo.h
@@ -0,0 +1,4 @@
+struct sysinfo {
+  uint64 freemem;   // Available memory (bytes)
+  uint64 nproc;     // Number of active processes (state != UNUSED)
+};
```

새 파일 `kernel/sysinfo.h`는 `struct sysinfo`만 정의한다. `uint64`는 `kernel/types.h`의 형이므로, 이 헤더를 포함하는 파일은 그 앞에 `types.h`를 포함해야 한다. `new file mode 100644` 줄은 이 파일이 새로 생겼음을 나타낸다.

```diff
diff --git a/kernel/sysproc.c b/kernel/sysproc.c
index 419e727..77c0f20 100755
--- a/kernel/sysproc.c
+++ b/kernel/sysproc.c
@@ -6,6 +6,7 @@
 #include "spinlock.h"
 #include "proc.h"
 #include "vm.h"
+#include "sysinfo.h"
 
 uint64
 sys_exit(void)
@@ -107,3 +108,32 @@ sys_uptime(void)
   release(&tickslock);
   return xticks;
 }
+
+// sys_trace: set the trace mask for the current process
+uint64
+sys_trace(void)
+{
+  int mask;
+
+  argint(0, &mask);
+  myproc()->trace_mask = mask;
+  return 0;
+}
+
+// sys_sysinfo: collect system information and copy to user space
+uint64
+sys_sysinfo(void)
+{
+  uint64 addr;
+  struct sysinfo info;
+  struct proc *p = myproc();
+
+  argaddr(0, &addr);
+
+  info.freemem = freemem_count();
+  info.nproc = proc_count();
+
+  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
+    return -1;
+  return 0;
+}
```

203번 줄은 `sysproc.c`에 `sysinfo.h`를 포함한다. 212\~221번 줄의 `sys_trace()`는 배포된 패치의 TODO 주석 세 단계를 그대로 코드로 옮긴 것이다. 지역 변수 `mask`에 첫 번째 인자를 읽고, 호출한 프로세스의 `trace_mask`에 저장한 뒤 0을 반환한다.

223\~239번 줄의 `sys_sysinfo()`는 사용자 버퍼의 주소를 `addr`에 읽고(231번 줄), 커널 스택의 지역 변수 `info`를 두 보조 함수로 채운 뒤(233\~234번 줄), `copyout()`으로 호출한 프로세스의 페이지 테이블 `p->pagetable`을 거쳐 `sizeof(info)`, 곧 16바이트를 복사한다(236번 줄). 복사가 실패하면 -1을, 성공하면 0을 반환한다.

---

<br>

## 8. 사용자 쪽 파일

240\~426번 줄은 `user/trace_test.c`를 새 파일로 더하는 부분이다. 내용은 배포된 `Original/trace_test.c`의 181줄과 같으며, 테스트 다섯 개를 차례로 실행한다. 테스트 1은 `fork`를, 테스트 2는 파이프를 통한 `read`와 `write`를, 테스트 3은 `getpid`, `fork`, `write`를 함께 추적하고, 테스트 4는 마스크를 켠 뒤 `fork()`한 자식의 `write`가 추적되는지 확인하며, 테스트 5는 `trace(0)` 뒤에 아무것도 출력되지 않는지 본다. 실행 결과와 출력의 특징은 [과제 해설](Assignment-Explanation.ko.md)의 6절에 있다.

```diff
diff --git a/user/user.h b/user/user.h
index ac84de9..6d5fdb4 100755
--- a/user/user.h
+++ b/user/user.h
@@ -24,6 +24,9 @@ int getpid(void);
 char* sys_sbrk(int,int);
 int pause(int);
 int uptime(void);
+int trace(int);
+struct sysinfo;
+int sysinfo(struct sysinfo*);
 
 // ulib.c
 int stat(const char*, struct stat*);
diff --git a/user/usys.pl b/user/usys.pl
index c5d4c3a..08df9f2 100755
--- a/user/usys.pl
+++ b/user/usys.pl
@@ -42,3 +42,5 @@ entry("getpid");
 entry("sbrk");
 entry("pause");
 entry("uptime");
+entry("trace");
+entry("sysinfo");
```

`user/user.h`의 435번 줄은 `trace()`의 원형이다. 436번 줄의 `struct sysinfo;`는 구조체의 내용 없이 이름만 알리는 전방 선언으로, 437번 줄의 원형이 그 구조체에 대한 포인터를 받을 수 있게 한다. 사용자 프로그램이 필드에 접근하려면 `kernel/sysinfo.h`를 따로 포함해야 한다. `user/usys.pl`의 449\~450번 줄은 빌드 때 `usys.S`에 `trace`와 `sysinfo`의 스텁을 만든다. 각 스텁은 `li a7, SYS_trace`처럼 번호를 a7에 넣고 `ecall`과 `ret`을 실행한다.

---

<br>

## 요약

| 개념 | 핵심 요약 |
|:-----|:-----|
| 패치 형식 | `diff --git`이 파일을, `@@` 머리가 변경 위치를 나타내고, `+` 줄은 더한 줄, 공백으로 시작하는 줄은 문맥이다. |
| 배포된 패치와의 관계 | 번호, 스텁, 선언, 필드, `trace`의 원형과 테이블 항목은 주어졌고, `sys_trace()` 본문, 이름 배열, 추적 출력, 상속, 초기화를 작성했으며, `sysinfo` 관련 변경은 모두 더했다. |
| syscall()의 추적 | 처리 함수의 반환값을 a0에 저장한 직후 `trace_mask & (1 << num)`을 검사하고, 이름 배열과 `(int)` 캐스트로 안내문의 형식을 출력한다. |
| 보조 함수 | `freemem_count()`는 `kmem.lock`을 잡은 채 free list를 세고, `proc_count()`는 슬롯마다 락을 잡고 `UNUSED`가 아닌 프로세스를 센다. |
| sys_sysinfo | 커널 스택의 구조체를 채워 `copyout()`으로 사용자 페이지 테이블을 거쳐 16바이트를 복사하고, 실패하면 -1을 반환한다. |
| 사용자 쪽 | `usys.pl`의 항목이 a7에 번호를 넣고 `ecall`하는 스텁을 만들고, `user.h`의 전방 선언으로 `sysinfo()`가 구조체 포인터를 받는다. |

---

<br>

## 점검 문제

1. **hunk 머리:** `@@ -282,6 +283,9 @@ kfork(void)`에서 두 쌍의 숫자가 각각 무엇을 뜻하며, 새 파일의 줄 수가 3 늘어난 이유를 쓰라.

   <details>
   <summary>정답</summary>

   `-282,6`은 원래 파일의 282번 줄부터 6줄, `+283,9`는 새 파일의 283번 줄부터 9줄을 뜻한다. 시작 줄이 1 늘어난 것은 앞쪽 `freeproc()`에서 한 줄을 더했기 때문이다. 이 변경 단위는 주석, 대입문, 빈 줄의 3줄을 더하므로 6줄이 9줄이 된다.

   </details>

2. **이름 배열의 첨자:** `syscall_names[]`를 `[SYS_fork] "fork"`처럼 첨자를 지정하지 않고 문자열만 나열하면 어떤 문제가 생기는지 쓰라.

   <details>
   <summary>정답</summary>

   첨자를 지정하지 않으면 첫 문자열이 0번 칸에 들어가므로, 1번부터 시작하는 시스템 콜 번호와 이름이 하나씩 어긋난다. 예를 들어 `syscall_names[SYS_fork]`, 곧 1번 칸은 두 번째 문자열인 `"exit"`가 된다. 첨자를 지정하면 번호와 이름의 대응이 나열 순서와 상관없이 정해진다.

   </details>

3. **헤더 순서:** `sysproc.c`에서 `#include "sysinfo.h"`를 `#include "types.h"`보다 앞에 두면 어떻게 되는지 쓰라.

   <details>
   <summary>정답</summary>

   `sysinfo.h`의 필드 형 `uint64`는 `types.h`가 정의하므로, 그보다 앞에서 포함하면 컴파일러가 `uint64`를 알지 못해 컴파일 오류가 난다. 패치는 기존 헤더들의 맨 뒤에 포함하여 이 문제를 피한다.

   </details>
