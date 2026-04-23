# 운영체제 중간고사 총정리

---

## 목차

- [제0장. 시험 대비 사용 가이드](#제0장-시험-대비-사용-가이드)
- [제1장. 운영체제(Operating System)란 무엇인가 (W01)](#제1장-운영체제란-무엇인가-w01)
- [제2장. 프로세스(Process)의 탄생과 생애 (W02)](#제2장-프로세스의-탄생과-생애-w02)
- [제3장. 프로세스 간 통신(Inter-Process Communication, IPC) — IPC (W03)](#제3장-프로세스-간-통신--ipc-w03)
- [제4장. 스레드(Thread)와 동시성(Concurrency)의 기초 (W04)](#제4장-스레드와-동시성의-기초-w04)
- [제5장. 암묵적 스레딩(Implicit Threading)과 스레딩 이슈 (W05)](#제5장-암묵적-스레딩과-스레딩-이슈-w05)
- [제6장. CPU 스케줄링 기본 알고리즘 (W06)](#제6장-cpu-스케줄링-기본-알고리즘-w06)
- [제7장. 고급 스케줄링 — 멀티프로세서와 실시간 (W07)](#제7장-고급-스케줄링--멀티프로세서와-실시간-w07)
- [제8장. 큰 그림 — 7주간의 개념들이 어떻게 맞물리는가](#제8장-큰-그림--7주간의-개념들이-어떻게-맞물리는가)
- [제9장. 시험 직전 체크리스트](#제9장-시험-직전-체크리스트)

---

# 제0장. 시험 대비 사용 가이드

## 0.1 공부 순서

1. **1회독**: 1장부터 8장까지 읽으며 각 절 끝의 **"한 줄 요약"** 확인
2. **2회독**: 각 장의 예시/계산을 **직접 손으로** 풀어봄
3. **3회독**: 8장(개념 연결 고리)를 여러 번 읽어 **왜 이 개념들이 함께 존재하는지** 체화
4. **시험 전날**: 9장 체크리스트로 놓친 부분 확인

## 0.2 "깊은 이해" 답안을 쓰는 6요소 템플릿

교수님께서 보너스 점수를 주신다는 **깊은 이해 답안** 은 다음 6요소를 포함합니다.

| 요소 | 설명 | 예시 |
|---|---|---|
| ① 정의 | 한글 + 영어 + 1문장 | "프로세스(Process)는 실행 중인 프로그램이다." |
| ② 왜 필요한가 | 대안·반례·동기 | "만약 프로세스 추상화가 없다면 한 번에 하나의 프로그램만 실행 가능하다." |
| ③ 메커니즘 | 어떻게 작동하는지 | "PCB에 PC·레지스터·메모리 정보를 저장, 컨텍스트 스위치(Context Switch)로 교체..." |
| ④ 구체 예시 | 코드·계산·실무 | "브라우저를 켜면 새 프로세스 생성, 각각 독립된 메모리..." |
| ⑤ 한계/트레이드오프 | 비용·제약 | "그러나 프로세스 간 통신(Inter-Process Communication, IPC)은 IPC 필요, 오버헤드 큼" |
| ⑥ 다른 개념과 연결 | 다른 단원과 연관 | "이 한계가 스레드(Thread) 도입의 동기이며, 4주차의..." |

## 0.3 본 문서에 사용하는 기호

- 🎯 **핵심 개념** — 반드시 이해할 부분
- 💡 **직관** — 비유나 간단한 설명
- ⚠️ **함정** — 자주 틀리는 지점
- 🔗 **연결** — 다른 단원과의 관계
- 🧪 **예시/계산** — 손으로 따라가야 하는 부분

---

# 제1장. 운영체제(Operating System)란 무엇인가 (W01)

## 1.1 우리가 풀어야 할 문제 — 왜 OS가 필요한가

컴퓨터 하드웨어는 **CPU, 메모리, 디스크, 네트워크 카드, 키보드** 같은 장치들의 집합입니다. 사용자는 워드프로세서를 쓰고, 음악을 듣고, 브라우저를 열고 싶어합니다. **그런데 이 두 세계를 직접 연결하면 엄청난 문제가 생깁니다.**

- 🎯 **문제 1**: 한 번에 하나의 프로그램만 실행 가능. 워드를 쓰려면 브라우저를 꺼야 함.
- 🎯 **문제 2**: 각 프로그램이 하드웨어를 직접 제어 → 잘못된 프로그램 하나가 디스크를 삭제할 수 있음.
- 🎯 **문제 3**: 프로그램이 메모리를 제멋대로 쓰면 다른 프로그램의 데이터가 깨짐.
- 🎯 **문제 4**: 프로그래머마다 각 하드웨어의 세부 명령을 다 알아야 함 — 이식성 제로.

**운영체제(OS, Operating System)** 는 이 모든 문제를 해결하기 위한 **중간 계층** 입니다.

```
┌───────────────────────────┐
│  응용 프로그램 (Word, Browser...) │
├───────────────────────────┤
│  운영체제 (OS)                │  ← 우리가 배우는 것
├───────────────────────────┤
│  하드웨어 (CPU, 메모리, 디스크...)│
└───────────────────────────┘
```

## 1.2 커널(Kernel) — OS의 심장 🎯

**OS 의 핵심은 "커널(Kernel)" 이다.** 시험에서 "OS 란 무엇인가?" 라고 물으면 반드시 이 단어가 답에 들어가야 합니다.

**커널의 세 가지 정의적 특성**:

1. **항상 실행 중인 프로그램** — 컴퓨터가 부팅된 순간 시작되어 전원이 꺼질 때까지 절대 종료되지 않는 유일한 프로그램
2. **하드웨어 자원 관리자** — 모든 하드웨어 자원(CPU, 메모리, I/O) 을 관리하고 응용 프로그램(Application Program)에 서비스 제공
3. **특권 실행** — 사용자 모드(User Mode)에서는 접근할 수 없는 특권 명령어(Privileged Instruction)를 실행할 수 있는 유일한 프로그램

💡 **비유 — OS 는 정부와 같다**: 정부가 직접 상품을 만들거나 서비스를 제공하지 않듯이, OS 도 직접 유용한 일을 하지 않습니다. 정부는 치안·인프라·통화를 제공해 시장이 굴러가게 하고, OS 는 자원 관리·보호·스케줄링을 제공해 응용 프로그램이 굴러가게 합니다.

**OS 구성 요소의 분류**:

| 구분 | 실행 모드 | 예시 |
|---|---|---|
| **커널(Kernel)** | 커널 모드(Kernel Mode)에서만 | 프로세스(Process) 관리, 메모리 관리, 파일시스템 |
| **시스템 프로그램(System Program)** | 사용자 모드 | 셸(bash), 컴파일러(gcc), ls, GUI |
| **응용 프로그램(Application)** | 사용자 모드 | 워드프로세서, 브라우저, 게임 |

시스템 프로그램은 **OS 와 함께 배포되지만** 사용자 모드에서 실행된다는 점에서 커널과 구분됩니다. 예를 들어 셸(bash) 은 리눅스에 기본 포함되지만, 셸이 하는 일(명령어 파싱, 입출력 리디렉션)은 모두 사용자 모드에서 벌어집니다. 셸이 시스템 콜(System Call) `fork()`, `exec()`, `dup2()` 등을 호출해야 비로소 커널이 일합니다.

## 1.3 OS 의 두 역할 — 자원 할당자 vs 제어 프로그램

OS 의 역할은 두 가지 상호 보완적인 관점에서 볼 수 있습니다.

### 1.3.1 자원 할당자 (Resource Allocator)

**"한정된 자원을 공정하고 효율적으로 어떻게 나눌까?"**

관리 대상:
- **CPU 시간**: 어떤 프로세스(Process)가 언제 실행? → CPU 스케줄링 (6~7주차)
- **메모리 공간**: 각 프로세스에 어느 영역을 할당? → 메모리 관리 (11~12주차)
- **저장장치 & I/O**: 디스크 접근 순서, 장치 공유 → I/O 스케줄링

핵심 트레이드오프:
- **공정성(Fairness)** vs **효율성(Efficiency)** — 모든 프로세스에게 공평한 기회를 주면서도 전체 처리량(Throughput)을 최대화해야 함

### 1.3.2 제어 프로그램 (Control Program)

**"사용자 프로그램이 시스템을 오용하지 못하게 어떻게 막을까?"**

담당 기능:
- **실행 관리** — 프로그램의 시작·정지·일시 중단 제어
- **오류 방지** — 0으로 나누기, 권한 없는 메모리 접근 등 불법 연산 포착
- **격리 보장** — 프로세스(Process) A 의 버그가 프로세스 B 나 커널(Kernel)을 건드리지 못하게 보호

🔗 **연결**: 이 두 역할은 분리되지 않습니다. 자원을 공정하게 나누려면(자원 할당자) 반드시 한 프로세스가 자원을 독점하지 못하도록 강제할 수 있어야 하고(제어 프로그램), 그러려면 하드웨어가 두 모드를 지원해야 합니다 → 1.4 이중 모드(Dual-Mode).

## 1.4 이중 모드(Dual-Mode) 동작 — OS 가 자신을 지키는 방법 🎯

**가장 중요한 질문**: 만약 어떤 사용자 프로그램이 "디스크 전체를 삭제하라" 는 명령을 실행하려 하면 어떻게 막을까요?

답: **하드웨어가 물리적으로 막습니다.** 이것이 **이중 모드(Dual-Mode Operation)** 의 핵심입니다.

### 1.4.1 두 개의 실행 모드

| 모드 | 모드 비트 | 실행 주체 | 권한 |
|---|---|---|---|
| **커널 모드(Kernel/Supervisor Mode)** | 0 | OS 커널(Kernel) | 모든 명령어, 모든 메모리 접근 |
| **사용자 모드(User Mode)** | 1 | 응용 프로그램(Application Program) | 제한된 명령어, 제한된 메모리 |

**모드 비트의 물리적 위치**: CPU 의 **상태 레지스터(Status Register)** 에 저장됩니다. RISC-V 에서는 `sstatus` 레지스터의 SPP(Supervisor Previous Privilege) 비트가 이 역할을 합니다.

### 1.4.2 특권 명령어 (Privileged Instruction)

**특권 명령어(Privileged Instruction)**는 잘못 사용하면 시스템에 해를 끼칠 수 있는 CPU 명령어로, **커널 모드(Kernel Mode)에서만 실행 가능**합니다. 사용자 모드(User Mode)에서 시도하면 **하드웨어가 물리적으로 차단**하고 **트랩(Trap)** 을 발생시킵니다.

대표적 특권 명령어:
1. **I/O 명령어** — 디스크·네트워크 카드 직접 접근
2. **인터럽트(Interrupt) 제어** — 인터럽트 활성화/비활성화
3. **타이머 설정** — CPU 타이머 제어
4. **메모리 관리** — 페이지 테이블(Page Table) 변경
5. **모드 비트 변경** — 사용자 ↔ 커널(Kernel) 전환

### 1.4.3 왜 하드웨어가 강제해야 하는가

⚠️ **함정 주의**: 소프트웨어로 이 규칙을 강제할 수 없습니다. 만약 OS 가 "나쁜 명령어를 실행하지 마세요" 라고 경고만 하고 하드웨어가 강제하지 않는다면, 악성 프로그램이 경고를 무시하면 끝입니다. **하드웨어가 물리적으로 차단** 하기 때문에 소프트웨어 트릭으로는 절대 우회할 수 없습니다.

### 1.4.4 트랩 (Trap) — 의도적 인터럽트(Interrupt)

**트랩(Trap)** 은 소프트웨어가 **의도적으로 발생시키는 인터럽트(Interrupt)** 입니다. 프로그램이 커널(Kernel)의 도움을 요청할 때 사용합니다.

트랩 발생 메커니즘:
1. 사용자 모드(User Mode) 프로그램이 특권 명령어(Privileged Instruction) 실행을 시도하거나, 시스템 콜(System Call)을 호출
2. CPU 가 **트랩을 발생** 시킴
3. 하드웨어가 **자동으로** 모드 비트를 0(커널 모드(Kernel Mode)) 로 설정
4. 제어가 커널의 **트랩 핸들러(trap handler)** 로 이동
5. 커널이 요청을 처리한 후 반환
6. 하드웨어가 모드 비트를 1(사용자 모드) 로 복원

🎯 **중요한 특성**:
- **하드웨어 강제** — 모드 전환은 하드웨어가 수행, 소프트웨어가 임의로 모드를 바꿀 수 없음
- **모든 커널 진입은 추적 가능** — 특권 명령어 시도 또는 시스템 콜을 통해서만 진입, 보안 감시에 유리

💡 **비유 — 은행 창구**: 은행 직원(커널 모드)은 금고에 접근 가능, 고객(사용자 모드)은 창구(시스템 콜)를 통해서만 서비스 요청 가능. 고객이 창구를 뛰어넘으려 하면 보안요원이 즉시 제지 = 트랩 발생.

**한 줄 요약**: OS 가 자신을 보호하기 위해 CPU 가 두 모드를 갖고, 사용자 모드에서 특권 명령을 시도하면 하드웨어가 트랩으로 강제 차단한다.

## 1.5 시스템 콜(System Call) — 사용자와 커널(Kernel) 사이의 유일한 문 🎯

이중 모드(Dual-Mode)가 장벽이라면, **시스템 콜(System Call)** 은 장벽을 합법적으로 통과하는 **유일한 통로** 입니다.

### 1.5.1 시스템 콜(System Call)이란

**정의**: 사용자 프로그램이 OS 서비스를 요청하는 유일한 메커니즘.

**왜 필요한가**:
- 사용자 프로그램은 특권 명령어(Privileged Instruction)를 직접 실행할 수 없음
- 그러나 파일을 열고, 메모리를 할당하고, 화면에 출력하려면 결국 특권 명령이 필요
- → **커널(Kernel)에 대신 일을 시키는 표준 프로토콜** 이 필요 = 시스템 콜(System Call)

💡 **비유 — 레스토랑**: 손님은 주방(커널)에 들어갈 수 없고, 웨이터(시스템 콜)가 주문을 받아 주방에 전달. 손님이 만질 수 없는 장비(하드웨어)로 요리(서비스)를 만든 후 손님에게 갖다 줌.

### 1.5.2 시스템 콜(System Call) 호출의 전체 흐름 (RISC-V 기준)

`open("file.txt", O_RDONLY)` 호출의 **단계별 상세 경로**:

```
[1] 사용자 프로그램
       ↓ printf() 와 비슷하게 보이지만 다름
[2] C 라이브러리의 open() 래퍼 함수
       ↓ 시스템 콜 번호를 a7 레지스터에 설정
       ↓ 인자를 a0, a1, ... 레지스터에 설정
[3] ecall 명령어 실행 (RISC-V 트랩 명령)
       ↓ 하드웨어가 자동으로:
       ↓   - 현재 PC 를 sepc 에 저장
       ↓   - 모드 비트를 커널 모드로 전환
       ↓   - stvec 에 설정된 트랩 핸들러 주소로 점프
[4] 커널의 usertrap() 트랩 핸들러 진입
       ↓
[5] syscall() — a7 값을 읽어 시스템 콜 테이블 조회
       ↓ 예: a7 = 15 (SYS_open)
[6] sys_open() 핸들러 실행
       ↓ 파일 디스크립터 할당, inode 탐색 등
[7] 결과를 a0 에 담음 (예: fd = 3)
       ↓
[8] sret 명령 실행 (RISC-V 반환)
       ↓ 하드웨어가 자동으로:
       ↓   - 모드 비트를 사용자 모드로 복원
       ↓   - PC 를 sepc 값으로 복원
[9] C 라이브러리 래퍼가 a0 을 반환 값으로 전달
       ↓
[10] 사용자 프로그램이 fd 를 받음
```

### 1.5.3 ecall 명령어의 특별함

⚠️ **혼동 주의**: `ecall` 은 단순한 C 함수가 아닙니다. **특수한 CPU 수준의 명령** 으로, 하드웨어에 다음을 명령합니다:
1. 사용자 코드 실행 중단
2. 즉시 OS 커널(Kernel)로 제어권 이전
3. 모드 비트를 커널 모드(Kernel Mode)로 설정

이 전환은 **원자적(atomic)** 으로 이루어지며, 중간에 사용자 프로그램이 개입할 방법이 없습니다.

### 1.5.4 시스템 콜(System Call) 번호와 인자 전달

각 시스템 콜(System Call)은 고유한 번호로 식별됩니다.

| 아키텍처 | 번호 레지스터 | 인자 레지스터 | 반환값 레지스터 |
|---|---|---|---|
| **RISC-V** | `a7` | `a0 ~ a6` (최대 7개) | `a0` |
| **x86-64** | `rax` | `rdi, rsi, rdx, r10, r8, r9` | `rax` |

**시스템 콜 테이블(Syscall Table)**: 커널(Kernel) 내부 배열. 번호를 인덱스로 해당 핸들러 함수 포인터를 찾음.

```c
// xv6 kernel/syscall.c 의 간략 모델
static uint64 (*syscalls[])(void) = {
    [SYS_fork]    sys_fork,
    [SYS_exit]    sys_exit,
    [SYS_wait]    sys_wait,
    [SYS_pipe]    sys_pipe,
    [SYS_read]    sys_read,
    [SYS_open]    sys_open,
    ...
};

void syscall(void) {
    int num = myproc()->trapframe->a7;  // 시스템 콜 번호
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        myproc()->trapframe->a0 = syscalls[num]();  // 결과를 a0 에
    } else {
        myproc()->trapframe->a0 = -1;  // 알 수 없는 syscall
    }
}
```

### 1.5.5 libc 래퍼 vs 시스템 콜(System Call)

⚠️ **자주 혼동**: `printf()` 는 시스템 콜(System Call)이 **아닙니다** — C 라이브러리 함수입니다. `write()` 가 실제 시스템 콜입니다.

연쇄 호출:
1. `printf("Hello\n")` — 사용자 모드(User Mode), C 라이브러리
2. 내부적으로 버퍼링 수행
3. 버퍼가 차거나 개행이 오면 `write(1, buf, len)` 시스템 콜 호출
4. 커널(Kernel) 진입, 실제로 파일/콘솔에 쓰기
5. 사용자 모드 복귀

### 1.5.6 성능 — 시스템 콜(System Call)은 비싸다

🧪 **예시**: `cp in.txt out.txt` 의 내부
- `open("in.txt", O_RDONLY)` — 1회
- `open("out.txt", O_WRONLY|O_CREAT)` — 1회
- 루프로 반복:
  - `read(fd_in, buffer, 4096)` — 읽기
  - `write(fd_out, buffer, bytes)` — 쓰기
- `close()` × 2

만약 파일이 4MB 라면 약 **1000회 read + 1000회 write = 2000회 시스템 콜(System Call)**. 각 호출마다 모드 전환 오버헤드(수백 ns ~ µs)가 발생합니다.

🔗 **연결**: 이것이 **버퍼링**, **DMA**, **메모리 맵 파일(mmap)**, **sendfile** 같은 최적화가 중요한 이유입니다. 12주차 가상 메모리(Virtual Memory)의 mmap 이 이 비용을 획기적으로 줄입니다.

**한 줄 요약**: 사용자 프로그램은 특권 명령을 직접 못 쓰므로, 시스템 콜 = ecall 로 트랩(Trap)을 일으켜 커널(Kernel)에 대신 일을 시킨다. 모든 커널 진입은 이 단일 통로를 거친다.

## 1.6 컴퓨터 시스템의 동작 — 인터럽트(Interrupt)와 DMA

### 1.6.1 버스 기반 아키텍처

```
┌─────┐      버스(Bus)       ┌──────┐
│ CPU │ ←───────────────→  │ 메모리 │
└─────┘                     └──────┘
   ↑
   ├─── 디스크 컨트롤러
   ├─── USB 컨트롤러
   └─── 네트워크 컨트롤러
```

**버스(Bus)**: CPU, 메모리, I/O 장치 사이의 데이터 통신 경로.

### 1.6.2 인터럽트 (Interrupt)

**정의**: 외부 장치가 CPU 에 **"작업 완료" 또는 "처리 요청"** 을 알리는 비동기 신호.

**왜 필요한가**:
- 대안인 **폴링(Polling)** 은 비효율적 — CPU 가 주기(Period)적으로 장치 상태를 확인해야 함
- 인터럽트(Interrupt)는 장치가 준비됐을 때만 CPU 를 깨움 → CPU 자원 절약

**인터럽트 처리 흐름**:
1. 장치가 작업을 완료하면 **인터럽트 라인** 을 통해 신호 전송
2. CPU 가 현재 명령 실행 완료 후 인터럽트 확인
3. **인터럽트 벡터 테이블(IVT: Interrupt Vector Table)** 참조 — 각 인터럽트 번호를 해당 처리 루틴 주소로 매핑
4. 해당 **인터럽트 서비스 루틴(ISR: Interrupt Service Routine)** 실행
5. 처리 후 이전 실행으로 복귀

**인터럽트 특성**:
- **비동기적(Asynchronous)** — 현재 실행 중인 명령과 무관하게 언제든 발생
- **병렬성(Parallelism) 확보** — CPU 가 다른 일을 하는 동안 I/O 장치가 독립적으로 작업

### 1.6.3 DMA (Direct Memory Access)

**정의**: CPU 개입 없이 메모리 ↔ I/O 장치 간에 직접 데이터를 전송하는 메커니즘.

**왜 필요한가**:
- DMA 없으면 CPU 가 **바이트 단위로** 데이터를 옮겨야 함 → 1GB 파일 읽기 시 수십억 번의 CPU 명령 필요
- DMA 가 있으면 CPU 는 "어디서 어디로 몇 바이트" 만 지시하고 다른 일 수행 가능

**DMA 절차**:
1. CPU → DMA 컨트롤러에 명령 (소스 주소, 대상 주소, 바이트 수)
2. CPU 는 다른 작업 수행
3. DMA 컨트롤러가 데이터 전송 수행 (CPU 개입 없음)
4. 전송 완료 시 DMA 컨트롤러가 **인터럽트(Interrupt) 발생**
5. CPU 가 ISR 실행하여 완료 처리

🔗 **연결**: 현대 시스템의 고성능 I/O(SSD, 네트워크 카드, GPU) 는 모두 DMA 기반입니다.

## 1.7 저장 장치 계층 (Storage Hierarchy)

### 1.7.1 속도-용량-비용의 삼각형

| 수준 | 장치 | 용량 | 접근 시간 | 관리 주체 |
|---|---|---|---|---|
| 0 | 레지스터 | < 1 KB | ~0.3 ns | 하드웨어 |
| 1 | L1/L2 캐시 | < 64 MB | ~1–25 ns | 하드웨어 |
| 2 | 주기(Period)억장치(RAM) | < 64 GB | ~100 ns | **OS** |
| 3 | SSD | < 4 TB | ~50 µs | **OS** |
| 4 | HDD | < 20 TB | ~5 ms | **OS** |

- **위로 갈수록** 빠르고 비싸고 작다
- **아래로 갈수록** 느리고 저렴하고 크다

### 1.7.2 지역성 원리 (Locality) — 왜 캐시가 작동하는가

만약 프로그램의 메모리 접근이 완전히 무작위라면, 어떤 캐시도 효과가 없습니다. 다행히 실제 프로그램은 두 가지 **지역성** 을 보입니다.

🎯 **시간적 지역성(Temporal Locality)**: 최근 접근한 데이터는 곧 다시 접근될 가능성이 높다.
- 예: `for (i=0; i<100; i++) sum += i` — `sum` 을 100번 연속 접근

🎯 **공간적 지역성(Spatial Locality)**: 접근한 데이터 근처의 데이터가 곧 접근될 가능성이 높다.
- 예: 배열 `a[0]`, `a[1]`, `a[2]`, ... 를 순차 접근

💡 **책 읽기 비유**: 50페이지를 읽은 후 다음으로 51페이지를 읽을 가능성이 높고(공간적), 방금 읽은 50페이지를 다시 볼 수도 있음(시간적). 캐시는 이 예측 가능성을 활용함.

🔗 **연결**: 이 원리가 **TLB(주소 변환 캐시), 페이지 캐시(파일 시스템 캐시), 버퍼 캐시** 등 모든 캐싱 기법의 전제입니다.

## 1.8 OS 구조 — 어떻게 커널(Kernel)을 설계할까

OS 커널(Kernel)을 어떻게 구성할지에 대한 네 가지 대표 패러다임이 있습니다.

### 1.8.1 모놀리식 (Monolithic)

모든 OS 기능(프로세스(Process) 관리, 메모리 관리, 파일 시스템, 드라이버 등) 을 **하나의 커널(Kernel) 바이너리** 에 포함.

- **장점**: 기능 간 통신이 **함수 호출** 이므로 빠름. 모든 코드를 전역적으로 최적화 가능.
- **단점**: **한 버그가 전체 시스템을 다운시킴** (파일 시스템 버그 → 모든 프로세스 영향). 수백만 줄의 커널 코드 → 디버깅 어려움.
- **예시**: Linux, 전통 UNIX, Solaris

### 1.8.2 마이크로커널 (Microkernel)

**최소한의 커널(Kernel)만 커널 공간에 유지** (IPC, 스케줄링, 기본 메모리 관리). 대부분의 OS 서비스(파일 시스템 서버, 네트워크 스택, 드라이버) 는 **사용자 공간의 서버 프로세스(Process)** 로 실행.

- **장점**:
  - **고장 격리** — 파일 시스템 서버가 충돌해도 커널은 안전, 재시작만 하면 됨
  - **검증 용이** — 커널이 작으므로 증명 검증(formal verification) 가능
  - **이식성** — 플랫폼 의존성 최소
- **단점**:
  - **컨텍스트 스위칭(Context Switching) 오버헤드** — 서비스 간 통신도 IPC 필요
  - **구현 복잡도** — 모든 기능을 독립 프로세스로 분리해 조율해야 함
- **예시**: Mach, QNX, L4

💡 **왜 마이크로커널(Microkernel)이 이론적 우수성에도 주류가 못 되었나**: 답은 **성능** 입니다. 서비스 간 통신마다 유저↔커널 전환 비용이 쌓여 느려집니다.

### 1.8.3 하이브리드 (Hybrid)

모놀리식(Monolithic)과 마이크로커널(Microkernel)의 **장점 결합**. 성능이 중요한 부분은 커널(Kernel)에, 안정성이 중요한 부분은 사용자 공간으로.

- **예시**: macOS(Mach 마이크로커널 + BSD Unix), Windows

### 1.8.4 적재 가능 모듈 (Loadable Modules)

커널(Kernel)의 **핵심만** 정적으로 컴파일, 나머지는 **동적 모듈** 로 로드/언로드.

- **예시**: Linux LKM (Loadable Kernel Module), `insmod`, `rmmod`

**현실**: 대부분 현대 OS 는 실용적 타협으로 **하이브리드(Hybrid)** + **적재 가능 모듈(Loadable Module)** 구조를 사용합니다.

## 1.9 컨텍스트 스위칭 (Context Switching)

**정의**: 현재 실행 중인 프로세스(Process)의 상태를 저장하고, 다른 프로세스의 상태를 복원하여 실행을 재개하는 것.

### 1.9.1 컨텍스트에 포함되는 것

1. **CPU 레지스터 값** — 범용 레지스터, 상태 레지스터
2. **프로그램 카운터(PC)** — 다음 실행 명령 주소
3. **스택 포인터(SP)**
4. **페이지 테이블(Page Table)** — 메모리 매핑 정보
5. **파일 디스크립터 테이블**
6. **커널(Kernel) 메타데이터** — 프로세스(Process) 상태, 우선순위 등

### 1.9.2 절차

```
현재 프로세스 P1
  │
  ├─ [1] P1 의 레지스터, PC, SP 등을 메모리에 저장 (P1 의 PCB)
  │
  ├─ [2] 메모리에서 P2 의 상태를 읽어 CPU 레지스터에 복원 (P2 의 PCB)
  │
  └─ [3] PC 를 P2 가 마지막 중단 지점으로 설정
       ↓
     P2 실행 재개
```

### 1.9.3 오버헤드와 최적화

**비용 요인**:
- 레지스터 저장/복원 (수십 개 레지스터)
- 캐시 무효화 (새 프로세스(Process) 데이터가 캐시에 없을 수 있음)
- TLB 플러시 (가상-물리 주소 변환 캐시 초기화)

**하드웨어 최적화**:
- **다중 레지스터 세트** (SPARC 레지스터 윈도) — 레지스터를 메모리에 저장할 필요 없음
- **태그드 TLB (ASID)** — 주소 공간 ID 를 TLB 엔트리에 태그 → 주소 공간 전환 시 TLB 전체 플러시 불필요

🔗 **연결**: 마이크로커널(Microkernel)이 느린 근본 이유는 컨텍스트 스위칭(Context Switching)이 자주 발생하기 때문. 스레드(Thread) 간 전환이 프로세스 간보다 빠른 이유도 **TLB 플러시 불필요** 때문 (4장에서 상세).

## 1.10 xv6 — 교육용 운영체제(Operating System)

**xv6** 는 MIT 에서 만든 **Unix 계열 교육용 OS** 입니다. 본 수업에서 학기 내내 읽고 수정합니다.

| 특성 | 내용 |
|---|---|
| 아키텍처 | RISC-V |
| 언어 | C |
| 코드 줄 수 | **약 10,000줄** |
| 범위 | 프로세스(Process), 가상 메모리(Virtual Memory), 파일시스템, 셸 |

**왜 xv6 가 좋은 교육 자료인가**:
- 실제 Linux 는 수천만 줄 → 읽기 불가능
- xv6 는 1만 줄 → 전체를 읽을 수 있으면서 핵심 OS 개념을 모두 포함

**실행 방법**:
```bash
git clone https://github.com/mit-pdos/xv6-riscv
cd xv6-riscv
make qemu    # QEMU 에뮬레이터에서 xv6 부팅
```

**필요 도구**:
- **RISC-V 크로스 컴파일러** (`riscv64-unknown-elf-gcc`) — x86/ARM 호스트에서 RISC-V 바이너리 생성
- **QEMU** (`qemu-system-riscv64`) — RISC-V 하드웨어 에뮬레이션

---

## 1장 정리 🎯

**1장의 큰 그림**: OS = 커널(Kernel). 커널은 하드웨어 자원을 관리하며 응용 프로그램(Application Program)에 서비스를 제공한다. 하드웨어 차원의 이중 모드(Dual-Mode)로 자신을 보호하고, 시스템 콜(System Call) = 트랩(Trap) 을 통해 사용자 프로그램에게 합법적 통로를 제공한다. 인터럽트(Interrupt)와 DMA 로 I/O 효율을 확보하고, 저장 계층은 지역성 원리로 작동한다.

**시험 출제 예상**:
1. "OS 란 무엇인가" — 커널 중심 정의
2. "이중 모드가 없다면" — 보안 취약성
3. "시스템 콜의 전체 경로" — 단계별 서술
4. "트랩과 인터럽트 차이" — 동기/비동기
5. "모놀리식(Monolithic) vs 마이크로커널(Microkernel) 트레이드오프"

---

# 제2장. 프로세스(Process)의 탄생과 생애 (W02)

## 2.1 프로세스(Process) ≠ 프로그램 — 근본적 구분 🎯

이 구분은 시험에 매우 자주 나오는 **개념의 기초**입니다.

| 프로그램(Program) | 프로세스(Process) |
|---|---|
| 디스크에 저장된 **정적 파일** | 메모리에 적재되어 **실행 중인 실체** |
| 수동적(passive) 개체 | 능동적(active) 개체 |
| `.exe`, `a.out`, `.class` | PC·레지스터·메모리·상태를 보유 |
| 하나의 프로그램 | 여러 프로세스로 동시 실행 가능 |

🧪 **실제 예시**: 한 번 설치한 **크롬 브라우저 프로그램(1개)** 을 세 명의 사용자가 각자 실행 → **3개의 프로세스**. 각 프로세스의 텍스트(코드) 영역은 동일하지만 데이터·힙·스택은 각자 독립입니다.

### 2.1.1 왜 구분이 중요한가

- 같은 프로그램이 **여러 프로세스(Process)** 로 실행 가능 → 격리(isolation) 의 기본
- 프로세스 자체가 **다른 코드의 실행 환경** 이 될 수 있음 (JVM 이 그 예)
- **다중화(Multiplexing)**: 하나의 CPU 를 여러 프로세스가 빠르게 교대로 실행하여 "동시 실행 착각" 을 만듦

## 2.2 프로세스(Process)의 메모리 레이아웃 🎯

프로세스(Process)는 메모리에 다음과 같은 구조로 배치됩니다.

```
[↑ 높은 주소]
┌──────────────────────┐
│   Stack              │ ← 아래로 성장 (함수 호출 시)
│   [지역 변수·인자·반환]  │
├──────────────────────┤
│      ↕ (빈 공간)      │
├──────────────────────┤
│   Heap               │ ← 위로 성장 (malloc, new 시)
│   [동적 할당]         │
├──────────────────────┤
│   BSS                │ ← 초기화 안 된 전역 변수 (0으로 자동 초기화)
├──────────────────────┤
│   Data               │ ← 초기화된 전역 변수
├──────────────────────┤
│   Text               │ ← 실행 코드 (읽기 전용)
└──────────────────────┘
[↓ 낮은 주소]
```

### 2.2.1 각 영역의 상세

| 영역 | 저장 내용 | 크기 | 주소 성장 |
|---|---|---|---|
| **Text (코드)** | 기계어 명령 | 고정, 읽기 전용 | — |
| **Data** | 초기화된 전역 변수 (`int x = 5;`) | 고정 | — |
| **BSS** | 초기화 안 된 전역 변수 (`int y;`) | 고정, 0으로 자동 초기화 | — |
| **Heap** | `malloc()`, `new` 로 할당한 동적 메모리 | 동적 | ↑ 위로 |
| **Stack** | 함수 호출 프레임 (지역 변수, 매개변수, 반환 주소) | 동적 | ↓ 아래로 |

### 2.2.2 왜 이렇게 설계했나

🎯 **스택과 힙이 서로를 향해 성장하는 이유**: 빈 공간 한 덩어리를 두 영역이 양쪽에서 필요한 만큼 사용 → 메모리 효율 ↑. OS 는 두 영역이 충돌하지 않도록 감시합니다.

🎯 **BSS 를 Data 와 분리하는 이유 (중요!)**:
- 초기화된 변수는 **실행 파일에 초기값을 저장** 해야 함 (`int x = 1000000` → 파일에 8바이트 기록)
- 초기화 안 된 변수는 **"BSS 영역 크기는 8바이트"** 정보만 저장하면 됨
- OS 가 프로그램 로드 시 BSS 영역을 0으로 자동 초기화
- **결과: 실행 파일이 훨씬 작아짐**

💡 **Text 라는 이름의 유래**: 초기 어셈블리 언어의 `.text` 지시자에서 유래. "프로그램의 본문(text)" 이라는 비유.

### 2.2.3 스택 프레임 (활성화 레코드)

함수가 호출될 때마다 스택에 **스택 프레임(Stack Frame, Activation Record)** 이 쌓입니다.

스택 프레임의 구성:
- 함수의 **지역 변수**
- **매개변수**
- **반환 주소** (함수 종료 후 돌아갈 PC)
- **이전 스택 프레임 포인터** (호출자 복원용)

**LIFO 규칙**: A가 B를 호출하고 B가 C를 호출하면, 반환 순서는 C → B → A. 스택 포인터(Stack Pointer, SP)는 높은 주소에서 낮은 주소로 이동합니다. 이것이 재귀 호출과 중첩 호출을 자연스럽게 지원합니다.

### 2.2.4 실제 C 프로그램 분석

```c
#include <stdio.h>
#include <stdlib.h>

int x;           // 초기화 안 됨 → BSS
int y = 15;      // 초기화됨 → Data

int main(int argc, char *argv[]) {
    int *values; // 지역 변수 → Stack
    int i;       // 지역 변수 → Stack

    values = (int *)malloc(sizeof(int) * 5);  // 동적 할당 → Heap
    for (i = 0; i < 5; i++)
        values[i] = i;
    return 0;
}
```

컴파일 후 `size ./프로그램` 실행 시:
```
text   data   bss   dec   hex
1158   284    8     1450  5aa
```

- `text` — 실행 코드 섹션 크기
- `data` — 초기화된 전역 데이터
- `bss` — 초기화 안 된 전역 데이터

🔗 **연결**: 이 메모리 레이아웃은 **11~12주차 가상 메모리(Virtual Memory)** 에서 페이지 테이블(Page Table)과 직접 연관됩니다. 각 영역이 다른 페이지에 매핑되고, 코드 영역은 읽기 전용 페이지 권한이 부여됩니다.

## 2.3 프로세스(Process) 상태와 상태 전이 🎯

프로세스(Process)는 실행 중에 다음 5가지 상태 중 하나를 가집니다.

```
                      ┌──── admitted ────┐
                      │                  ↓
   ┌────┐         ┌──────┐         ┌────────┐         ┌──────────┐
   │New │────────→│Ready │←────────│Running │────────→│Terminated│
   └────┘         └──────┘         └────────┘         └──────────┘
                      ↑     dispatch    ↓
                      │                  │
              interrupt │                │ I/O or event wait
                      │     I/O done     │
                      │   ┌──────────────┘
                      │   ↓
                   ┌──────────┐
                   │ Waiting  │
                   └──────────┘
```

### 2.3.1 다섯 가지 상태

| 상태 | 설명 | 조건 |
|---|---|---|
| **New** | 생성 중 | OS 가 아직 메모리에 적재하지 않음 |
| **Ready** | CPU 를 기다리는 중 | 실행 준비 완료, 스케줄러 선택 대기 |
| **Running** | 명령어 실행 중 | CPU 코어에서 활발히 실행 |
| **Waiting** | 이벤트 대기 | I/O 완료, 신호 수신 등 대기 |
| **Terminated** | 종료됨 | 프로세스(Process) 테이블에서 제거 대기 |

**중요 원칙**:
- **단일 코어**에서 어느 순간 **Running 은 하나** 뿐
- 여러 프로세스가 동시에 Ready 또는 Waiting 가능
- **멀티코어**에서는 코어 수만큼 Running 가능

### 2.3.2 상태 전이의 원인 — 자주 나오는 시험 포인트

| 전이 | 원인 | 누가 결정 |
|---|---|---|
| **New → Ready** | `admitted` (OS 가 생성 승인, 메모리 할당) | OS |
| **Ready → Running** | `scheduler dispatch` (스케줄러가 선택) | 스케줄러 |
| **Running → Ready** | `interrupt` (타이머 등 **비자발적**) | 타이머 하드웨어 |
| **Running → Waiting** | `I/O or event wait` (**자발적**) | 프로세스(Process) 자신 |
| **Waiting → Ready** | `I/O or event completion` | I/O 컨트롤러·ISR |
| **Running → Terminated** | `exit()` | 프로세스 자신 |

🎯 **자주 출제되는 구분**:
- **Running → Ready (비자발)**: 타임 슬라이스(Time Slice) 만료 → OS 가 강제로 CPU 회수
- **Running → Waiting (자발)**: 프로세스가 I/O 요청 → 스스로 CPU 양보

### 2.3.3 타임 슬라이스(Time Slice)

**타임 슬라이스(Time Slice) = 퀀텀(Quantum)**: OS 가 각 프로세스(Process)에 할당하는 고정 CPU 시간 (일반적으로 10~100ms). 퀀텀 만료 시 타이머 인터럽트(Interrupt)로 Running → Ready 전이가 발생합니다.

🔗 **연결**: 라운드 로빈(Round Robin, RR) 스케줄링의 핵심 파라미터 (6장).

## 2.4 프로세스 제어 블록 (PCB) 🎯

**PCB(Process Control Block)** = 커널(Kernel)이 각 프로세스(Process)에 대해 유지하는 **자료구조**. 다른 이름은 **TCB(Task Control Block)**. 프로세스를 **시작하거나 재시작** 하는 데 필요한 모든 정보를 담습니다.

### 2.4.1 PCB 의 주요 필드

| 필드 | 설명 |
|---|---|
| **프로세스(Process) 상태** | new, ready, running, waiting, terminated |
| **프로그램 카운터 (PC)** | 다음 실행 명령의 주소 |
| **CPU 레지스터** | 누산기, 인덱스 레지스터, SP, 범용, 조건 코드 |
| **CPU 스케줄링 정보** | 우선순위, 스케줄링 큐 포인터 |
| **메모리 관리 정보** | base/limit 레지스터 값, 페이지 테이블(Page Table) |
| **계정 정보** | PID, 사용된 CPU 시간, 프로세스 번호 |
| **I/O 상태 정보** | 할당된 I/O 장치, **열린 파일 목록**(fd table) |

### 2.4.2 가장 중요한 필드

- **프로그램 카운터(PC)** — 컨텍스트 스위치(Context Switch) 후 어디서 재개할지 결정
- **프로세스(Process) 상태** — 실행 가능한지 판단

### 2.4.3 베이스/리밋 레지스터(Limit Register)와 메모리 보호(Memory Protection)

**베이스(Base) 레지스터**: 프로세스(Process)가 접근 가능한 메모리의 **시작 주소**
**리밋(Limit) 레지스터**: 해당 프로세스 메모리의 **크기(범위)**

**하드웨어 강제 메모리 보호(Memory Protection)**: CPU 가 메모리 접근 시 주소가 `[base, base+limit)` 범위 내인지 자동 검사. 범위 밖 접근 → 트랩(Trap) 발생 → OS 가 프로세스 종료.

🔗 **연결**: 이 단순한 방식은 11~12주차 가상 메모리(Virtual Memory)의 페이지 테이블(Page Table)로 진화합니다.

### 2.4.4 PCB 와 컨텍스트 스위치(Context Switch)

컨텍스트 스위치(Context Switch) 발생 시:
1. **현재 프로세스(Process) 상태 저장**: PC, 모든 레지스터, 메모리 관리 정보 → 현재 프로세스의 PCB 에 저장
2. **새 프로세스 상태 복원**: 새 프로세스의 PCB 에서 PC, 레지스터 값 → CPU 에 로드

⚠️ **PCB 없이는 컨텍스트 스위칭(Context Switching)이 불가능!** 중단된 프로세스를 나중에 정확히 이어갈 수 없습니다.

### 2.4.5 Linux 의 task_struct

Linux 는 PCB 를 `task_struct` 구조체로 구현합니다. 위치: `<include/linux/sched.h>`.

```c
struct task_struct {
    long state;                    // 프로세스 상태
    struct sched_entity se;        // 스케줄링 정보
    struct task_struct *parent;    // 부모 프로세스 포인터
    struct list_head children;     // 자식 프로세스 리스트 (이중 연결)
    struct files_struct *files;    // 열린 파일 (fd 테이블)
    struct mm_struct *mm;          // 주소 공간 (메모리 정보)
    // ... 수백 개 더 ...
};
```

- 모든 활성 프로세스(Process)는 **이중 연결 리스트** 로 관리 (O(1) 삽입/삭제)
- `current` — 현재 실행 중인 프로세스의 `task_struct` 포인터

## 2.5 프로세스(Process) 스케줄링 개요

### 2.5.1 왜 스케줄링이 필요한가

**다중 프로그래밍(Multiprogramming)**: 항상 어떤 프로세스(Process)가 실행되도록 해 **CPU 이용률(CPU Utilization) 극대화**. CPU 가 I/O 완료를 기다리는 동안 다른 프로세스 실행.

**시분할(Time Sharing)**: CPU 를 여러 프로세스 간에 빈번하게 전환해 사용자에게 **상호작용성** 제공.

### 2.5.2 I/O 바운드(I/O Bound) vs CPU 바운드(CPU Bound)

| 유형 | 특성 | 예시 |
|---|---|---|
| **I/O 바운드(I/O Bound)** | 계산보다 I/O 시간이 길다 | 웹 브라우저, DB 앱 |
| **CPU 바운드(CPU Bound)** | 계산 시간이 길다 | 과학 계산, 이미지 처리 |

**적절한 혼합**이 중요:
- I/O 바운드만 있으면 → 프로세스(Process)가 자주 I/O 대기 → CPU 유휴
- CPU 바운드만 있으면 → 장치 유휴
- 혼합 시 CPU·장치 모두 높은 이용률

### 2.5.3 스케줄링 큐

**레디 큐(Ready Queue)**: CPU 를 기다리는 모든 프로세스(Process)의 연결 리스트.

**대기 큐(Wait/Device Queue)**: 특정 이벤트 대기 프로세스의 큐. 장치별로 여러 개 존재.

```
신규 프로세스
     ↓ 메모리 할당
 [Ready Queue] ────→ [CPU 스케줄러] ────→ Running
     ↑                                    ↓
     │                              타임 슬라이스 만료
     │                                    │
     │                              I/O 요청
     └─ I/O 완료 ←── [Device Queue] ←────┘
                     (디스크, 터미널 등)
```

## 2.6 fork() — 프로세스(Process) 생성 🎯

### 2.6.1 부모-자식 관계와 프로세스 트리(Process Tree)

**부모 프로세스(Parent)**: 다른 프로세스(Process)를 생성하는 프로세스.
**자식 프로세스(Child)**: 새로 생성된 프로세스.
**프로세스 트리(Process Tree)**: 부모-자식 관계로 형성되는 트리.

Linux 에서는 **systemd**(PID=1) 가 모든 사용자 프로세스의 **루트 부모**. 부팅 시 커널(Kernel)이 생성합니다.

```
[systemd] (PID=1)
   ├─ [sshd]
   │   └─ [user shell (bash)]
   │       ├─ [python]
   │       └─ [vim]
   ├─ [nginx]
   │   ├─ [worker 1]
   │   └─ [worker 2]
   └─ ...
```

### 2.6.2 fork() 의 동작

**정의**: `fork()` 는 호출하는 프로세스(Process)의 주소 공간을 **완전히 복제** 하여 자식 프로세스(Child Process)를 생성하는 시스템 콜(System Call).

```
┌──────────────────┐       ┌──────────────────┐       ┌──────────────────┐
│  부모 (PID 1000) │  fork()│  부모 (PID 1000) │       │  자식 (PID 1001) │
│  Text            │  ───→  │  Text            │  +    │  Text            │
│  Data: x=5       │        │  Data: x=5       │       │  Data: x=5       │
│  Heap            │        │  Heap            │       │  Heap            │
│  Stack           │        │  Stack           │       │  Stack           │
└──────────────────┘       └──────────────────┘       └──────────────────┘
                                                          ↑ 완전한 복사
```

### 2.6.3 fork() 의 반환 값 규약 ⚠️

```c
pid_t pid = fork();
```

| pid 값 | 의미 |
|---|---|
| **0** | 자식 프로세스(Child Process) (새로 생성된 쪽) |
| **> 0** | 부모 프로세스(Parent Process) (값은 자식의 PID) |
| **-1** | fork() 실패 |

**왜 이렇게 설계했나**:
- **부모가 자식 PID 를 받는 이유**: 자식을 관리하려면 PID 가 필요 (wait, kill)
- **자식이 0 을 받는 이유**: 자신의 PID 는 `getpid()` 로 알 수 있음. 부모의 PID 는 `getppid()` 로 알 수 있음. 0 이 가장 단순한 프로토콜.

### 2.6.4 COW (Copy-On-Write) 최적화

⚠️ **문제**: fork() 가 메모리를 **완전히 복사** 하면 비용이 큽니다. 게다가 자식이 곧바로 exec() 를 호출하면 복사한 메모리가 모두 버려집니다 → **극심한 낭비**.

🎯 **해결**: **COW(Copy-On-Write)**
- fork() 직후에는 부모와 자식이 **같은 물리 메모리 페이지 공유**
- 페이지 테이블(Page Table)의 플래그를 "읽기 전용" 으로 표시
- **누군가 쓰기를 시도하면** 그 시점에 페이지를 복사하고 플래그를 "읽기/쓰기" 로 변경
- 대부분의 페이지는 실제로 복사되지 않음

🔗 **연결**: 이 최적화로 fork() 비용이 거의 0 에 가까워지고, 특히 **fork + exec 패턴** 에서 절대적 효과를 냅니다.

## 2.7 exec() — 프로세스(Process) 이미지 교체 🎯

**정의**: 현재 프로세스(Process)의 메모리 이미지를 **새로운 프로그램으로 완전히 대체**.

### 2.7.1 exec() 의 특성

- **PID 는 변경되지 않음** — 같은 프로세스(Process), 다른 프로그램
- **성공 시 반환하지 않음** — 메모리가 덮어씌워지므로 이전 코드로 돌아갈 수 없음
- **실패 시만 반환** (-1) — 잘못된 경로, 권한 부족 등

```
Before exec():              After exec():
┌────────────┐              ┌────────────┐
│ Stack      │              │ Stack (new)│
├────────────┤              ├────────────┤
│ Heap       │   exec()     │ Heap (new) │
├────────────┤    ────→     ├────────────┤
│ Data (old) │              │ Data (new) │
├────────────┤              ├────────────┤
│ Text (old) │              │ Text (new) │
└────────────┘              └────────────┘
PID = 1234                   PID = 1234 (불변!)
```

### 2.7.2 exec() 함수 계열

C 라이브러리는 여러 변형을 제공합니다. 이름 규칙:
- **l** (list) — 가변 인자 리스트
- **v** (vector) — 문자열 배열
- **p** (path) — PATH 환경변수 검색
- **e** (environment) — 커스텀 환경 지정

| 함수 | 인자 형식 | 경로 | 환경 |
|---|---|---|---|
| `execl(path, arg0, arg1, ..., NULL)` | 가변 리스트 | 전체 경로 | 상속 |
| `execlp(file, arg0, arg1, ..., NULL)` | 가변 리스트 | **PATH 검색** | 상속 |
| `execv(path, argv[])` | 배열 | 전체 경로 | 상속 |
| `execvp(file, argv[])` | 배열 | **PATH 검색** | 상속 |

🧪 **가장 많이 쓰는 형태**:
```c
execlp("ls", "ls", "-l", NULL);
// 첫 "ls": 검색할 프로그램 이름
// 두 번째 "ls": argv[0], UNIX 관례로 프로그램 이름
// "-l": argv[1]
// NULL: argv 배열 끝 표시
```

## 2.8 fork + exec + wait 패턴 — UNIX 의 걸작 🎯

### 2.8.1 기본 패턴

```c
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        // 오류
        fprintf(stderr, "fork failed\n");
        return 1;
    } else if (pid == 0) {
        // 자식 프로세스
        printf("Child: pid=%d, parent=%d\n", getpid(), getppid());
        execlp("/bin/ls", "ls", "-l", NULL);
        // exec 성공 시 여기 도달 불가
        perror("exec failed");
        return 1;
    } else {
        // 부모 프로세스
        printf("Parent: my pid=%d, child pid=%d\n", getpid(), pid);
        wait(NULL);  // 자식 종료 대기
        printf("Child finished\n");
    }
    return 0;
}
```

### 2.8.2 UNIX 의 천재적 설계 — 왜 fork 와 exec 를 분리했나

🎯 **핵심 통찰**: `fork()` 와 `exec()` 를 분리함으로써 자식 프로세스(Child Process) 생성 후 **실행 전에** 자식 환경을 커스터마이즈할 기회가 생깁니다.

가능한 커스터마이즈:
- **파일 디스크립터 리디렉션**: `dup2()` 로 stdin/stdout 을 파이프(Pipe)나 파일로 연결 → 셸의 `>`, `<`, `|` 구현
- **환경 변수 변경**: `setenv()` 로 자식만의 환경 설정
- **작업 디렉토리 변경**: `chdir()`
- **사용자/그룹 변경**: `setuid()`, `setgid()` (권한 강등(Demotion))
- **시그널(Signal) 처리 초기화**

### 2.8.3 Windows CreateProcess() 와 대비

```c
// Windows 스타일 (하나의 함수로 모든 옵션 지정)
CreateProcess(
    NULL, "ls -l", NULL, NULL, FALSE,
    0, NULL, NULL, &startupInfo, &processInfo
);
```

Windows 는 생성 시점에 모든 옵션을 한꺼번에 지정해야 해 **유연성이 떨어집니다**.

| | UNIX fork+exec | Windows CreateProcess |
|---|---|---|
| 설계 철학 | 작은 도구 조합 | 큰 통합 함수 |
| 매개변수 수 | fork: 0개, exec: 몇 개 | 10개 이상 |
| 유연성 | 높음 (fork 후 자유 커스터마이즈) | 낮음 |
| 복잡성 | 조합으로 간단 | 단일 호출 복잡 |

### 2.8.4 wait() 의 역할

**wait()** 의 세 가지 일:
1. **자식 종료 대기** — 부모 블록
2. **자식의 종료 상태 수집** — 좀비(Zombie) 방지
3. **종료된 자식 PID 반환**

```c
int status;
pid_t child_pid = wait(&status);

if (WIFEXITED(status)) {
    int exit_code = WEXITSTATUS(status);
    printf("Child %d exited with status %d\n", child_pid, exit_code);
} else if (WIFSIGNALED(status)) {
    int sig = WTERMSIG(status);
    printf("Child killed by signal %d\n", sig);
}
```

| 매크로 | 용도 |
|---|---|
| `WIFEXITED(s)` | 정상 종료? |
| `WEXITSTATUS(s)` | 종료 코드 (0~255) |
| `WIFSIGNALED(s)` | 시그널(Signal)로 종료? |
| `WTERMSIG(s)` | 종료시킨 시그널 번호 |

## 2.9 좀비(Zombie)와 고아(Orphan) — 자주 출제되는 구분 🎯

### 2.9.1 좀비 프로세스 (Zombie)

**상태**: 자식이 **종료(exit)** 했지만, 부모가 아직 `wait()` 를 호출하지 않아 **프로세스(Process) 테이블에만 남아있는** 상태.

**특징**:
- 실행은 완전히 끝났음 (메모리·CPU 사용 안 함)
- 프로세스 테이블 엔트리만 차지 (PID, 종료 코드)
- **모든 프로세스는 종료 시 잠시 좀비(Zombie) 상태를 거침** (정상)

**문제**: 부모가 wait() 를 끝까지 호출하지 않으면 **좀비 누적** → 프로세스 테이블 슬롯 낭비 → 새 프로세스 생성 불가.

관찰:
```bash
ps aux | grep defunct
# 또는 상태 "Z"
```

### 2.9.2 고아 프로세스 (Orphan)

**상태**: 부모가 먼저 종료해서 **남겨진** 자식 프로세스(Child Process).

**특징**:
- 자식은 **아직 실행 중** (또는 대기 중)
- 부모가 사라진 상태

**Linux 의 처리**:
1. 커널(Kernel)이 부모의 모든 자식을 검색
2. 각 자식의 PPID(부모 PID) → **init/systemd(PID=1)로 변경**
3. 자식이 종료되면 systemd 가 주기(Period)적으로 `wait()` 호출해 정리

### 2.9.3 비교표

| | **좀비(Zombie)** | **고아(Orphan)** |
|---|---|---|
| 자식 상태 | 종료됨 (exit 호출) | 아직 실행 중 |
| 원인 | 부모가 wait() 미호출 | 부모가 먼저 종료 |
| 자원 | PT 슬롯만 낭비 | 정상 실행 중 |
| 해결 | 부모 wait() / 부모 사망시 init 입양 | init/systemd 자동 입양 |
| 누적 | 있음 (위험) | 없음 (자동 처리) |

### 2.9.4 좀비(Zombie) 제거 방법

⚠️ **중요**: 좀비(Zombie) 자체를 `kill` 로 죽일 수 없음 — 이미 죽었기 때문!

1. **부모에게 SIGCHLD 신호 전송** → 부모가 wait() 호출하도록 유도
2. **부모 프로세스(Parent Process) 강제 종료** → 좀비 부모가 init 로 변경 → 자동 정리
3. **예방**: POSIX 에서 `signal(SIGCHLD, SIG_IGN)` 설정 → OS 가 자동으로 좀비 회수

암기 도우미:
- 좀비: 죽었는데 프로세스(Process) 테이블에 눕지 않음 (좀비처럼)
- 고아(Orphan): 부모가 사라져서 systemd 가 입양 (고아원)

## 2.10 Chrome 멀티프로세스(Process) 아키텍처 — 실무 사례

단일 프로세스(Process) 브라우저의 문제:
- 한 탭 충돌 → 전체 브라우저 다운
- 한 사이트 악성 스크립트 → 모든 탭 영향

**Chrome 의 해결책**: 멀티프로세스

| 프로세스 | 역할 | 개수 |
|---|---|---|
| Browser | UI 관리, I/O, 프로세스 감시 | 1 |
| Renderer | 웹 페이지 렌더링 (HTML/CSS/JS) | 탭당 1 |
| Plug-in | 플러그인 실행 | 유형당 1 |
| GPU | 그래픽 가속 | 1 |

**이점**:
1. **격리** — 탭 1 충돌해도 탭 2,3 정상
2. **보안** — 렌더러는 **샌드박스**(제한된 권한 격리 환경)에서 실행
3. **응답성(Responsiveness)** — 한 탭 JS 무한 루프가 다른 탭 영향 안 줌
4. **병렬성(Parallelism)** — 멀티코어 활용

🔗 **연결**: 이 패턴은 "단일 프로세스 → 멀티프로세스" 로 성능·안정성·보안을 한번에 개선한 대표적 아키텍처 결정입니다.

---

## 2장 정리 🎯

**2장의 큰 그림**: 프로세스(Process)는 실행 중인 프로그램의 실체로, 고유한 메모리 레이아웃(Text/Data/BSS/Heap/Stack)과 상태(New/Ready/Running/Waiting/Terminated)를 갖는다. PCB 가 프로세스 정보를 담아 컨텍스트 스위치(Context Switch)를 가능하게 한다. fork/exec/wait 는 UNIX 의 우아한 프로세스 관리 메커니즘이며, 좀비(Zombie)와 고아(Orphan)는 라이프사이클의 특수 상태다.

---

# 제3장. 프로세스 간 통신(Inter-Process Communication, IPC) — IPC (W03)

## 3.1 왜 IPC 가 필요한가

현실에서는 프로세스(Process)가 **서로 협력** 해야 합니다.

### 3.1.1 독립 vs 협력 프로세스(Process)

- **독립 프로세스(Process)**: 다른 프로세스와 상호작용 없음
- **협력 프로세스**: 데이터 공유, 결과 주고받음

### 3.1.2 협력이 필요한 네 가지 이유

1. **정보 공유** — 복사-붙여넣기
2. **계산 속도 향상** — 작업 병렬화
3. **모듈성** — 기능별 프로세스(Process) 분리
4. **편의성** — 사용자가 여러 작업 동시 수행

### 3.1.3 실무 IPC 예시

| 애플리케이션 | IPC 메커니즘 |
|---|---|
| 셸 파이프(Pipe)라인 `cat log \| grep err` | 파이프 |
| Google Chrome | 공유 메모리(Shared Memory) + IPC |
| Docker | 명명 파이프(Named Pipe / FIFO), 소켓(Socket) |
| PostgreSQL | 공유 메모리 (버퍼 캐시) |
| Slack/Discord | 소켓 (WebSocket) |
| Android 앱 | Binder (RPC) |

## 3.2 두 가지 IPC 패러다임 🎯

### 3.2.1 공유 메모리(Shared Memory) vs 메시지 전달(Message Passing)

| | 공유 메모리(Shared Memory) | 메시지 전달(Message Passing) |
|---|---|---|
| 방법 | 공유 영역에 읽기/쓰기 | send/receive |
| 속도 | 빠름 (커널(Kernel) 미개입) | 느림 (매 호출 syscall) |
| 동기화 | **프로그래머 책임** | OS 관리 |
| 적합 | 대량 데이터, 로컬 | 소량, 분산 시스템 |
| 초기 설정 | `shm_open`, `mmap` | 링크 설정만 |
| 반복 비용 | 0 (일반 메모리처럼) | 높음 (시스템 콜(System Call)) |

### 3.2.2 공유 메모리(Shared Memory)의 메커니즘

공유 메모리(Shared Memory)는 OS 의 **가상 메모리(Virtual Memory) 페이지 테이블(Page Table) 조작** 으로 구현됩니다:
- 서로 다른 프로세스(Process)의 가상 주소가 **동일한 물리 프레임** 을 가리키도록 설정
- 매핑 후의 접근은 일반 메모리 접근과 동일 → 매우 빠름
- 하지만 동기화는 직접 해야 함

## 3.3 생산자(Producer)-소비자(Consumer) 문제

### 3.3.1 고전적 패턴

**생산자(Producer)**: 데이터 생성
**소비자(Consumer)**: 데이터 소비

실제 사례:
- 컴파일러(생산) → 어셈블러(소비)
- 웹 서버(생산) → 브라우저(소비)
- 키보드(생산) → OS(소비)

### 3.3.2 유한 버퍼(Bounded Buffer) 구현

```c
#define BUFFER_SIZE 10

typedef struct { /* ... */ } item;
item buffer[BUFFER_SIZE];
int in = 0;     // 다음 쓰기 위치
int out = 0;    // 다음 읽기 위치
```

**원형 배열** 메커니즘:
- 인덱스가 BUFFER_SIZE 에 도달하면 0 으로 순환
- **비어있음**: `in == out`
- **가득참**: `((in + 1) % BUFFER_SIZE) == out`
- **최대 저장**: `BUFFER_SIZE - 1` 개

🎯 **왜 한 슬롯을 비워두나**: 비어있음(in==out)과 가득참을 **구분** 하기 위해. 모든 슬롯을 쓰면 두 상태가 같아집니다.

**생산자(Producer) 코드**:
```c
while (true) {
    // next_produced 생산
    while (((in + 1) % BUFFER_SIZE) == out)
        ; // 바쁜 대기 — 버퍼 가득

    buffer[in] = next_produced;
    in = (in + 1) % BUFFER_SIZE;
}
```

**소비자(Consumer) 코드**:
```c
while (true) {
    while (in == out)
        ; // 바쁜 대기 — 버퍼 비어 있음

    next_consumed = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    // next_consumed 소비
}
```

### 3.3.3 바쁜 대기의 문제

⚠️ **바쁜 대기(Busy Wait)**: while 루프를 계속 돌며 조건 확인 → CPU 낭비.

🔗 **연결**: 9주차에서 **세마포어(Semaphore), 뮤텍스(Mutex), 조건 변수(Condition Variable)** 로 블로킹 방식을 도입합니다. 대기 중 프로세스(Process)를 슬립 상태로 만들어 CPU 해방.

## 3.4 메시지 전달(Message Passing)의 3가지 설계 축

### 3.4.1 네이밍: 직접 vs 간접

**직접 통신 (Direct)**:
- `send(P, message)` / `receive(Q, message)` — 상대 명시
- **대칭형**: 양쪽 모두 상대 지정 (1:1 고정)
- **비대칭형**: 수신자는 임의 프로세스(Process)로부터 (`receive(id, msg)`) — 서버 패턴
- 특징: 프로세스 식별자 변경 시 모든 참조 수정 필요 → **모듈성 제한**

**간접 통신 (Indirect)**:
- **메일박스(Mailbox) / 포트(Port)** 를 통해
- `send(A, msg)` / `receive(A, msg)`
- 두 프로세스가 공통 메일박스 공유 → 동적 환경에 유연
- 하나의 링크가 **둘 이상** 프로세스와 연관 가능

### 3.4.2 동기화: 블로킹 vs 넌블로킹

| | send | receive |
|---|---|---|
| 블로킹(동기) | 수신자가 받을 때까지 대기 | 메시지 도착까지 대기 |
| 넌블로킹(비동기) | 보내고 즉시 진행 | 가능하면 수신, 아니면 null |

**랑데부(Rendezvous)**: 블로킹 send + 블로킹 receive — 둘 다 서로 기다림. 생산자(Producer)-소비자(Consumer)가 매우 단순해짐.

### 3.4.3 버퍼링

| 버퍼링 | 용량 | 송신자 동작 |
|---|---|---|
| **용량 0** | 대기 메시지 없음 | 수신자가 받을 때까지 **반드시 대기** |
| **유한** | 최대 n 개 | 가득 차면 대기 |
| **무한** | 제한 없음 | 절대 대기 안 함 (비현실적) |

## 3.5 POSIX 공유 메모리(Shared Memory) — 실무 코드

### 3.5.1 3단계 API

```c
// 1단계: 공유 메모리 객체 생성
int fd = shm_open(name, O_CREAT | O_RDWR, 0666);

// 2단계: 크기 설정
ftruncate(fd, 4096);

// 3단계: 주소 공간에 매핑
char *ptr = (char *)mmap(0, 4096, PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0);
```

### 3.5.2 각 함수 상세

- **`shm_open(name, flags, mode)`**: 공유 메모리(Shared Memory) 객체 생성/열기
  - `name` — 프로세스(Process)들이 같은 이름으로 접근
  - `O_CREAT | O_RDWR` — 없으면 생성, 읽기/쓰기
  - `0666` — 파일 권한 (rw-rw-rw-)

- **`ftruncate(fd, SIZE)`**: 크기 설정. 새 공유 메모리의 기본 길이가 0이므로 필수.

- **`mmap(addr, length, prot, flags, fd, offset)`**: 매핑
  - `prot`: `PROT_READ | PROT_WRITE`
  - `flags`: `MAP_SHARED` (변경사항 다른 프로세스에 보임) vs `MAP_PRIVATE`

### 3.5.3 전체 예제 — 생산자(Producer)/소비자(Consumer)

**생산자(Producer)**:
```c
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>

int main() {
    const int SIZE = 4096;
    const char *name = "OS";
    const char *message_0 = "Hello";
    const char *message_1 = "World!";

    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, SIZE);
    char *ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    sprintf(ptr, "%s", message_0);
    ptr += strlen(message_0);
    sprintf(ptr, "%s", message_1);

    return 0;
}
```

**소비자(Consumer)**:
```c
int main() {
    const int SIZE = 4096;
    const char *name = "OS";

    int fd = shm_open(name, O_RDONLY, 0666);
    char *ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);

    printf("%s", ptr);  // "HelloWorld!" 출력

    shm_unlink(name);
    return 0;
}
```

컴파일:
```bash
gcc producer.c -o producer -lrt  # -lrt: POSIX 실시간 라이브러리
```

### 3.5.4 실무 활용

| 시스템 | 사용 방식 |
|---|---|
| PostgreSQL | 연결 간 공유 버퍼 풀 (8KB 페이지 캐시) |
| Redis | fork 지속성 — 부모↔자식 COW 공유 |
| Chromium | 렌더러 ↔ GPU 텍스처 공유 |
| 비디오 편집기 | 디코더 ↔ 플레이어 프레임 공유 |

## 3.6 파이프(Pipe) — UNIX 철학의 정수

### 3.6.1 일반 파이프 (Ordinary Pipe)

**정의**: 두 프로세스(Process) 간 단방향 데이터 채널. 특수 파일로, `read()/write()` 로 접근.

```c
int fd[2];
pipe(fd);
// fd[0] = 읽기 끝 (READ_END)
// fd[1] = 쓰기 끝 (WRITE_END)
```

**특성**:
- **단방향** (양방향 통신에는 두 개 필요)
- 자식이 **fork()** 로 파이프(Pipe) 상속
- **부모-자식 관계 필요**

### 3.6.2 실전 예제

```c
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 25
#define READ_END  0
#define WRITE_END 1

int main() {
    char write_msg[BUFFER_SIZE] = "Greetings";
    char read_msg[BUFFER_SIZE];
    int fd[2];

    if (pipe(fd) == -1) {
        fprintf(stderr, "Pipe failed\n");
        return 1;
    }

    pid_t pid = fork();

    if (pid > 0) {
        // 부모 — 쓰기
        close(fd[READ_END]);
        write(fd[WRITE_END], write_msg, strlen(write_msg) + 1);
        close(fd[WRITE_END]);
    } else if (pid == 0) {
        // 자식 — 읽기
        close(fd[WRITE_END]);
        read(fd[READ_END], read_msg, BUFFER_SIZE);
        printf("read %s\n", read_msg);
        close(fd[READ_END]);
    }

    return 0;
}
```

### 3.6.3 파이프(Pipe) 필수 규칙 🎯

⚠️ **가장 중요한 규칙**: 사용하지 않는 끝은 **반드시 close** 해야 한다.

**이유**: EOF 는 **모든 write 끝이 닫혀야** read 쪽에서 발생한다. 닫지 않으면 읽기 쪽이 **영원히 블록**.

**메커니즘 상세**:
1. `fork()` 후 부모와 자식 모두 `fd[0]`, `fd[1]` 의 복사본을 가짐
2. 부모가 사용 안 할 `fd[READ_END]` 를 close 안 하면, 커널(Kernel)은 "아직 읽기 fd 가 열려 있다" 고 판단
3. 자식(읽기 쪽)도 쓰기 끝 `fd[WRITE_END]` 를 쓸 수 있는 상태
4. 결국 커널은 "쓰기 가능한 fd 가 있다" 고 보고 EOF 발생 안 함
5. read 는 **영구 블록**

⚠️ **`strlen() + 1` 의 의미**: `+1` 은 null 종결 문자 `\0` 포함을 위함. 이걸 빼먹으면 `printf("%s")` 가 문자열 끝을 모름.

### 3.6.4 셸 파이프(Pipe)라인 — `ls | wc -l` 구현

```c
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pipe(fd);

    // 첫 자식 — ls
    if (fork() == 0) {
        close(fd[0]);  // 읽기 끝 닫기
        dup2(fd[1], STDOUT_FILENO);  // stdout → 파이프
        close(fd[1]);
        execlp("ls", "ls", NULL);
        perror("ls");
        return 1;
    }

    // 둘째 자식 — wc
    if (fork() == 0) {
        close(fd[1]);  // 쓰기 끝 닫기
        dup2(fd[0], STDIN_FILENO);   // stdin → 파이프
        close(fd[0]);
        execlp("wc", "wc", "-l", NULL);
        perror("wc");
        return 1;
    }

    // 부모 — 두 fd 모두 닫아야 함! (중요!)
    close(fd[0]);
    close(fd[1]);

    wait(NULL);
    wait(NULL);
    return 0;
}
```

🎯 **핵심**: 부모도 파이프(Pipe)를 사용 안 하므로 **양쪽 모두 close**. 안 그러면 wc 가 EOF 를 못 받음.

### 3.6.5 명명 파이프 (Named Pipe, FIFO)

일반 파이프(Ordinary Pipe)의 한계(부모-자식 관계 필요)를 극복한 고급 파이프(Pipe).

| | 일반 파이프 | 명명 파이프(Named Pipe / FIFO) |
|---|---|---|
| 프로세스(Process) 관계 | 부모-자식 | **무관한 프로세스도 가능** |
| 수명 | 프로세스 종료 시 파괴 | 파일 시스템에 영구 존재 |
| 식별 | fd 숫자 | **파일 시스템 경로** |

```c
// 생성
mkfifo("/tmp/my_fifo", 0666);

// 쓰는 프로세스
int fd = open("/tmp/my_fifo", O_WRONLY);
write(fd, "Hello", 6);
close(fd);

// 읽는 프로세스 (별도 프로그램)
int fd = open("/tmp/my_fifo", O_RDONLY);
read(fd, buf, 6);
close(fd);
```

| OS | 특성 |
|---|---|
| UNIX | 반이중 (한 방향씩) |
| Windows | 전이중, 원격 머신 가능 |

### 3.6.6 dup2() — 입출력 리디렉션

```c
int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
dup2(fd, STDOUT_FILENO);  // stdout 을 output.txt 로 리디렉션
close(fd);

printf("Redirected!\n");  // output.txt 에 기록됨
```

**dup2(oldfd, newfd)**: newfd 가 oldfd 와 같은 파일을 가리키게 설정.

**표준 fd**:
- `STDIN_FILENO` = 0 (키보드)
- `STDOUT_FILENO` = 1 (화면)
- `STDERR_FILENO` = 2 (화면, 에러)

**셸의 `ls > file.txt` 구현**:
1. fork
2. 자식: `open("file.txt", O_WRONLY|O_CREAT|O_TRUNC)` → fd
3. `dup2(fd, STDOUT_FILENO)` → stdout 을 file.txt 로
4. `close(fd)`
5. `execlp("ls", "ls", NULL)` — ls 의 출력이 자동으로 file.txt 에

💡 **UNIX 철학**: "한 가지를 잘 하고, 파이프(Pipe)로 연결하라." 이 철학 덕분에 `find / -name "*.log" | xargs grep ERROR | sort -u` 같은 강력한 조합이 가능합니다.

## 3.7 소켓 (Socket)

### 3.7.1 소켓(Socket) = 통신의 끝점

**정의**: 네트워크 통신의 끝점. **IP 주소 + 포트 번호** 로 식별.

**잘 알려진 포트**:
| 포트 | 서비스 |
|---|---|
| 22 | SSH |
| 80 | HTTP |
| 443 | HTTPS |
| 53 | DNS |
| 25 | SMTP |
| 21 | FTP |

**루프백**: `127.0.0.1` — 같은 머신 자기 자신.

### 3.7.2 TCP vs UDP

| | TCP | UDP |
|---|---|---|
| 연결 | 연결 지향 | 비연결 |
| 신뢰성 | 보장 (순서, 재전송) | 없음 |
| 속도 | 느림 | 빠름 |
| 용도 | HTTP, SSH, DB | 스트리밍, 게임, DNS |

💡 **비유**:
- TCP = 전화 (연결 후 통화, 확인)
- UDP = 편지 (바로 보냄, 도착 보장 없음)

### 3.7.3 실무 소켓(Socket) 사용

| 앱 | 프로토콜 | 용도 |
|---|---|---|
| ChatGPT | WebSocket (TCP) | 토큰 스트리밍 |
| 멀티플레이어 게임 | UDP | 저지연 위치 |
| Zoom | UDP (WebRTC) | 실시간 오디오/비디오 |
| 웹 브라우저 | TCP (HTTP/S) | 페이지 로드 |
| DB 클라이언트 | TCP 또는 UNIX 소켓(Socket) | psql, mysql |

### 3.7.4 소켓(Socket)의 한계

소켓(Socket)은 **비구조화 바이트 스트림** 만 주고받음. 데이터 구조화는 앱의 책임 → **직렬화(Serialization)** 필요 (JSON, Protocol Buffers).

## 3.8 RPC (원격 프로시저 호출(Remote Procedure Call, RPC))

### 3.8.1 개념

**정의**: 네트워크 연결된 원격 시스템의 프로시저를 **로컬 함수처럼 호출** 하는 메커니즘.

### 3.8.2 구성 요소

- **스텁(Stub)**: 클라이언트 측 프록시. 실제 함수처럼 보이지만 내부에서 매개변수 마샬링(Marshalling) 후 네트워크 전송
- **스켈레톤(Skeleton)**: 서버 측. 수신 데이터 언마샬링 → 실제 함수 호출

### 3.8.3 마샬링(Marshalling)과 엔디안(Endianness)

**마샬링(Marshalling)**: 데이터를 네트워크 전송 적합 형식으로 변환.

**엔디안(Endianness)**: 멀티바이트 데이터의 저장 순서.

예: `0x12345678`
- **빅 엔디안**: `12 34 56 78` (네트워크 바이트 순서)
- **리틀 엔디안**: `78 56 34 12` (x86, RISC-V)

**문제**: 클라이언트(리틀)와 서버(빅)가 다른 아키텍처면 데이터 깨짐. 해결: **XDR(External Data Representation)** 같은 중립 형식으로 마샬링.

### 3.8.4 실행 흐름

```
클라이언트                                서버
──────                                ──────
1. foo(x, y) 호출
      ↓
2. 클라이언트 스텁: 매개변수 마샬링
      ↓
3. 네트워크 전송
      ↓
4. 서버 스켈레톤: 언마샬링
      ↓
5. 실제 foo() 실행
      ↓
6. 결과 마샬링 & 반송
      ↓
7. 클라이언트 스텁: 언마샬링
      ↓
8. 반환값 제공
```

### 3.8.5 실행 의미론 — 매우 중요

네트워크 패킷은 유실 가능 → 재전송 → 중복 실행 위험. 이를 처리하는 세 의미론:

| 의미론 | 설명 | 안전 용도 |
|---|---|---|
| **At-least-once** | ACK 까지 재전송, 중복 검사 X | **멱등(Idempotent) 연산만** (조회) |
| **At-most-once** | 타임스탬프로 중복 무시 | 일부 유실 허용 |
| **Exactly-once** | at-most-once + ACK | **금융 거래 필수** |

**멱등(Idempotent)** 여부가 관건:
- 멱등: 여러 번 실행해도 결과 같음 (잔액 조회)
- 비멱등: 실행 횟수가 결과에 영향 (송금)

**구현**: 고유 트랜잭션 ID. 서버가 받은 ID 를 기록, 중복 ID 시 이전 결과 반환.

### 3.8.6 현대 RPC — gRPC

- **마샬링(Marshalling)**: Protocol Buffers (이진, 컴팩트)
- **전송**: HTTP/2 (멀티플렉싱, 스트리밍)
- **스텁(Stub) 자동 생성**: 10+ 언어 지원
- 사용 기업: Netflix, Uber, Kubernetes

```protobuf
service WeatherService {
  rpc GetForecast (Location) returns (Forecast);
}
```

클라이언트는 `GetForecast()` 를 로컬처럼 호출, gRPC 가 마샬링·전송 자동 처리.

## 3.9 xv6 커널(Kernel) 분석 — 실습

### 3.9.1 struct proc — xv6 의 PCB

```c
struct proc {
  struct spinlock lock;
  enum procstate state;        // UNUSED → USED → RUNNABLE → RUNNING → ZOMBIE
  void *chan;                  // 슬립 채널 (SLEEPING 상태)
  int killed;                  // 대기 중인 kill 신호
  int xstate;                  // 부모에게 전달할 종료 상태
  int pid;                     // 프로세스 ID

  struct proc *parent;         // 부모 포인터

  uint64 kstack;               // 커널 스택 가상 주소
  uint64 sz;                   // 프로세스 메모리 크기
  pagetable_t pagetable;       // 유저 페이지 테이블
  struct trapframe *trapframe; // 저장된 유저 레지스터
  struct context context;      // 저장된 커널 레지스터
  struct file *ofile[NOFILE];  // 열린 파일
  struct inode *cwd;           // 현재 작업 디렉토리
  char name[16];               // 프로세스 이름
};
```

**중요 필드**:
- **trapframe**: 유저 ↔ 커널(Kernel) 전환 시 유저 레지스터 저장
- **context**: 스케줄러의 커널 ↔ 커널 전환 시 사용
- 둘이 분리된 이유: 서로 다른 전환 담당

### 3.9.2 xv6 fork() 단계

1. **allocproc()** — 새 슬롯, pid, kstack, trapframe 할당
2. **uvmcopy()** — 부모 페이지 테이블(Page Table) + 메모리 복사 (**가장 비싼 단계, 먼저 수행**)
3. **trapframe 복사** — 자식도 fork() 리턴 가능
4. **a0 = 0 설정** — 자식의 fork() 반환값을 0으로
5. **ofile[] 복사** — 열린 fd 상속
6. **parent 설정, state = RUNNABLE**
7. **부모에게 자식 PID 반환**

🎯 **4단계의 의미**: RISC-V 에서 `a0` 는 함수 반환값 레지스터. 자식의 `trapframe->a0 = 0` 으로 설정하면 자식이 유저 모드로 복귀 시 `a0` 에서 0 을 읽어 "나는 자식" 임을 인식. 부모는 그대로 자식 PID 를 반환받음. **같은 코드가 두 프로세스(Process)에서 서로 다른 값 반환**의 비밀.

---

## 3장 정리 🎯

**3장의 큰 그림**: 프로세스(Process)는 격리되어 있지만 협력이 필요하다. 공유 메모리(Shared Memory)(빠르나 동기화 필요)와 메시지 전달(Message Passing)(안전하나 느림) 이 두 패러다임. 파이프(Pipe)는 UNIX 철학의 정수이고, 명명 파이프(Named Pipe / FIFO)는 파일시스템 기반으로 무관한 프로세스도 연결. 소켓(Socket)은 네트워크로, RPC 는 로컬 함수처럼 원격 호출을 추상화.

---

# 제4장. 스레드(Thread)와 동시성(Concurrency)의 기초 (W04)

## 4.1 스레드(Thread)란 무엇인가

### 4.1.1 정의

**스레드(Thread)** = 프로세스(Process) 내 **CPU 이용의 기본 단위**. 프로세스가 "자원 단위" 라면, 스레드는 "실행 단위".

### 4.1.2 스레드(Thread)가 소유/공유하는 것

**각 스레드(Thread)가 독립 소유**:
| 자원 | 역할 |
|---|---|
| 스레드 ID | 고유 식별자 |
| 프로그램 카운터(PC) | 다음 실행 명령 |
| 레지스터 세트 | 스레드의 CPU 상태 |
| 스택 | 함수 호출·지역 변수 |

**같은 프로세스(Process)의 모든 스레드가 공유**:
| 자원 | 설명 |
|---|---|
| 코드(Text) | 프로그램 코드 |
| 데이터 | 전역 변수 |
| 힙 | 동적 할당 |
| 파일 디스크립터 | 열린 파일 |
| 시그널(Signal) 핸들러 | |

### 4.1.3 단일 스레드(Thread) vs 다중 스레드

```
단일 스레드:
┌─────────────┐
│ Code + Data │
├─────────────┤
│ PC Reg Stack│ ← 1개
└─────────────┘

다중 스레드:
┌─────────────────────────────────┐
│        Code + Data              │ ← 공유
├─────────────────────────────────┤
│ PC₀ Reg₀ Stk₀ │ PC₁ Reg₁ Stk₁ │ ...
└─────────────────────────────────┘
    T0            T1
```

## 4.2 프로세스(Process) vs 스레드(Thread) 상세 비교 🎯

| | 프로세스(Process) | 스레드(Thread) |
|---|---|---|
| **생성 비용** | 높음 (메모리 할당·페이지 테이블(Page Table)) | 낮음 (스택·레지스터만) |
| **컨텍스트 스위치(Context Switch)** | 느림 (TLB 플러시) | **빠름 (TLB 플러시 X)** |
| **메모리 공유** | 격리 (IPC 필요) | 자연 공유 (전역 변수) |
| **독립성** | 높음 (하나 죽어도 다른 생존) | 낮음 (하나 죽으면 전체 영향) |

### 4.2.1 TLB 와 속도 차이 — 중요한 이유

**TLB(Translation Lookaside Buffer)**: 가상→물리 주소 변환 캐시(하드웨어).

- **프로세스(Process) 간 전환**: 페이지 테이블(Page Table) 바뀜 → TLB 플러시 필요 → 새 TLB 미스 빈번
- **스레드(Thread) 간 전환**: 같은 페이지 테이블 → TLB 유효 → 플러시 불필요 → 빠름

🔗 **연결**: 이 차이가 스레드를 쓰는 가장 큰 이유 중 하나입니다.

## 4.3 멀티스레딩의 네 가지 이점 🎯

### 4.3.1 응답성 (Responsiveness)

UI 스레드(Thread)가 블로킹 중에도 사용자 입력에 반응.
- 오래 걸리는 작업을 별도 스레드에서 **비동기** 실행
- 웹 브라우저: 네트워크 스레드 + 렌더링 스레드 + 입력 스레드

### 4.3.2 자원 공유 (Resource Sharing)

같은 주소 공간이므로 **자동 공유** — IPC 없이 전역 변수로 직접 통신.

### 4.3.3 경제성 (Economy)

스레드(Thread) 생성·전환 비용이 프로세스(Process)보다 훨씬 낮음.
- 웹 서버: 요청당 프로세스(고비용) 대신 요청당 스레드(저비용)

### 4.3.4 확장성 (Scalability)

멀티프로세서/멀티코어에서 **진정한 병렬 실행** 가능.
- 단일 스레드(Thread) 프로세스(Process) = 코어 수와 관계없이 1코어만 사용
- 멀티 스레드 = 각 스레드를 다른 코어에 배치 → 처리량(Throughput) ↑

## 4.4 동시성(Concurrency) vs 병렬성(Parallelism) 🎯

### 4.4.1 핵심 정의

**동시성(Concurrency)**: 여러 작업이 **진행 중(making progress)** 인 상태.
- 단일 코어에서도 가능 (시분할(Time Sharing) 인터리빙)

**병렬성(Parallelism)**: 여러 작업이 **실제로 동시 실행**.
- 멀티코어에서만 가능

### 4.4.2 관계

**병렬성(Parallelism) ⊂ 동시성(Concurrency)**. 모든 병렬성은 동시성이지만, 역은 아님.

| | 단일 코어 | 멀티 코어 |
|---|---|---|
| 동시성 | O (인터리빙) | O |
| 병렬성 | **X** | O |

### 4.4.3 시각화

```
단일 코어 (시분할):
Core: [T1][T2][T3][T4][T1][T2]...
      ↑ 인터리빙으로 동시성 달성, 병렬성 없음

멀티 코어 (병렬):
Core1: [T1    ][T3    ][T1    ]...
Core2: [T2    ][T4    ][T2    ]...
       ↑ T1과 T2가 실제로 동시 실행
```

💡 **비유**:
- 동시성(Concurrency): 한 사람이 요리와 빨래를 번갈아 함
- 병렬성(Parallelism): 두 사람이 각각 요리와 빨래를 함

## 4.5 멀티코어 프로그래밍의 과제

1. **작업 식별**: 독립·분리 가능한 작업 찾기
2. **균형**: 각 코어에 동등한 작업량
3. **데이터 분할**: 충돌 최소화
4. **데이터 의존성**: 동기화 필요
5. **테스트·디버깅**: 비결정성, Heisenbug

**Heisenbug**: 디버거로 관찰하면 사라지는 버그 (타이밍 변경 때문).

## 4.6 암달의 법칙(Amdahl's Law) 🎯

### 4.6.1 공식

$$ \text{speedup} \le \frac{1}{S + \frac{1-S}{N}} $$

- **S**: 순차 실행 비율 (병렬화 불가 부분)
- **N**: 프로세싱 코어 수

### 4.6.2 의미 — 무한 코어의 한계

N → ∞ 에서 **speedup → 1/S**.

💡 **해석**: 아무리 코어를 늘려도 직렬 부분(S)이 **성능 향상 상한**을 결정.

### 4.6.3 수치 표

| S (순차 비율) | N=2 | N=4 | N=8 | N=16 | N→∞ |
|:---:|:---:|:---:|:---:|:---:|:---:|
| **5%** | 1.90 | 3.48 | 5.93 | 10.20 | **20** |
| **10%** | 1.82 | 3.08 | 4.71 | 7.27 | **10** |
| **25%** | 1.60 | 2.28 | 3.02 | 3.76 | **4** |
| **50%** | 1.33 | 1.60 | 1.78 | 1.88 | **2** |

### 4.6.4 구체 계산 예시

**S = 25% (75% 병렬)**:
- N=2: `1 / (0.25 + 0.75/2) = 1 / 0.625 ≈ 1.6배`
- N=4: `1 / (0.25 + 0.75/4) = 1 / 0.4375 ≈ 2.28배`
- N=8: `1 / (0.25 + 0.75/8) = 1 / 0.34375 ≈ 2.91배`
- N=∞: `1 / 0.25 = 4배` (최대)

### 4.6.5 실무 시사점

1. **직렬 부분 최소화** 가 코어 증가보다 중요
2. 병렬 프로그래밍의 **본질적 한계**
3. S = 50% 면 코어 수 무관 **최대 2배**만 가능

## 4.7 병렬성(Parallelism)의 두 유형

### 4.7.1 데이터 병렬성 (Data Parallelism)

**같은 연산을 분산 데이터에** 적용.

```
Array: [1, 2, ..., 1000]
Thread 0: 1~250 합산      (같은 연산)
Thread 1: 251~500 합산    (같은 연산)
Thread 2: 501~750 합산    (같은 연산)
Thread 3: 751~1000 합산   (같은 연산)
```

- 적합: 배열·행렬, 이미지 처리, 과학 계산
- 확장성(Scalability): 데이터 크기에 선형

### 4.7.2 작업 병렬성 (Task Parallelism)

**다른 연산을 여러 스레드(Thread)에** 배분.

```
Array: [1, 2, ..., 1000]
Thread 0: 평균 계산
Thread 1: 표준편차 계산
Thread 2: 최대값 찾기
Thread 3: 최소값 찾기
```

- 적합: 웹 브라우저, 마이크로서비스
- 확장성(Scalability): 작업 수에 의존

현실에서는 **하이브리드(Hybrid)** 가 일반적.

## 4.8 사용자 스레드(User Thread)와 커널 스레드(Kernel Thread)

### 4.8.1 두 레벨

**사용자 스레드(User Thread)**: 사용자 공간 라이브러리가 관리. 커널(Kernel) 미인식.
- 라이브러리가 자체 스레드(Thread) 테이블 유지
- 단점: 한 스레드 블로킹 → 전체 프로세스(Process) 블록

**커널 스레드(Kernel Thread)**: OS 커널이 직접 관리. 커널의 스케줄러가 인식.
- 장점: 한 스레드 블로킹 시 다른 스레드 계속 실행
- 현대 OS 가 기본 지원

## 4.9 멀티스레딩 모델 🎯

### 4.9.1 다대일 (Many-to-One)

여러 사용자 스레드(User Thread) → 1 커널 스레드(Kernel Thread).

- **단점 (치명적)**:
  - 한 스레드(Thread) 블로킹 시스템 콜(System Call) → **전체 프로세스(Process) 블록**
  - **병렬성(Parallelism) 불가** (한 번에 한 커널 스레드만 스케줄)
  - 멀티코어에서 1코어만 사용
- 구식 Solaris, 초기 Java Green Threads — **거의 미사용**

### 4.9.2 일대일 (One-to-One) — 현대 표준

사용자 스레드(User Thread) ↔ 커널 스레드(Kernel Thread) 1:1.

- **장점**:
  - 한 스레드(Thread) 블록해도 다른 스레드 실행 가능
  - 진정한 병렬성(Parallelism)
- **단점**: 스레드 수 제한 (커널(Kernel) 리소스)
- **Linux, Windows** 표준

### 4.9.3 다대다 (Many-to-Many)

N 사용자 스레드(User Thread) ↔ M 커널 스레드 (M ≤ N).

- **장점**: 유연성, 넌블로킹, 병렬성(Parallelism)
- **단점**: 구현 복잡 (업콜(Upcall) 메커니즘)
- Go 고루틴, Java 21+ 가상 스레드(Thread)가 현대적 재등장

### 4.9.4 두 수준 (Two-Level)

다대다(Many-to-Many) + 중요 스레드(Thread) 1:1 바인딩. 구 Solaris, IRIX — 현재 거의 미사용.

### 4.9.5 최종 비교

| 모델 | 매핑 | 장점 | 단점 | 현재 |
|---|---|---|---|---|
| 다대일(Many-to-One) | N:1 | 관리 효율 | **블로킹 전파, 병렬 X** | ✗ |
| **일대일(One-to-One)** | 1:1 | 병렬, 안정 | 스레드(Thread) 수 제한 | **표준 ✓** |
| 다대다(Many-to-Many) | N:M | 유연, 병렬+넌블로킹 | 구현 복잡 | 제한적 |
| 두 수준(Two-Level) | N:M + 1:1 | 유연+우선 보장 | 매우 복잡 | 거의 미사용 |

🎯 **왜 일대일이 표준이 됐나**: 프로세싱 코어가 늘면서 "커널 스레드(Kernel Thread)를 아끼자" 는 다대다의 명분이 약해졌고, 일대일의 **단순성·안정성** 이 이겼습니다.

## 4.10 Pthreads — 표준 API

POSIX Pthreads = IEEE 1003.1c 표준. **명세** 이지 구현은 OS 설계자의 자유.

### 4.10.1 핵심 API

```c
int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);

int pthread_join(pthread_t thread, void **retval);
void pthread_exit(void *retval);
int pthread_attr_init(pthread_attr_t *attr);
```

### 4.10.2 pthread_create 분석

4개 인자:
1. **`pthread_t *thread`** — 출력: 새 스레드(Thread) ID
2. **`const pthread_attr_t *attr`** — 속성 (NULL = 기본값)
3. **`void *(*start_routine)(void*)`** — 스레드 함수 포인터
4. **`void *arg`** — 함수에 전달할 인자 (제네릭 포인터)

**함수 시그니처 `void *(*)(void *)`**: 제네릭 포인터 사용으로 **모든 타입** 전달 가능.

### 4.10.3 실전 예제 — 정수 합산

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sum;  // 전역 — 모든 스레드 공유

void *runner(void *param) {
    int upper = atoi((char *)param);
    sum = 0;
    for (int i = 1; i <= upper; i++)
        sum += i;
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, runner, argv[1]);
    pthread_join(tid, NULL);

    printf("sum = %d\n", sum);
    return 0;
}
```

실행 `./a.out 5`:
```
sum = 15
```

### 4.10.4 흔한 버그 — 인자 전달 ⚠️

```c
// ❌ 위험: 모든 스레드가 같은 &i 주소 공유
for (int i = 0; i < 4; i++) {
    pthread_create(&t[i], NULL, f, &i);
}
```

**문제**: `pthread_create` 가 비동기이므로 스레드(Thread)가 `*(int*)arg` 를 읽을 때쯤 `i` 가 이미 변경될 수 있음 → race condition.

```c
// ✅ 올바름: 각 스레드에 별도 저장소
int ids[4];
for (int i = 0; i < 4; i++) {
    ids[i] = i;
    pthread_create(&t[i], NULL, f, &ids[i]);
}
```

## 4.11 경쟁 조건(Race Condition)과 동기화 🎯

### 4.11.1 경쟁 조건 (Race Condition)

```c
global_sum += array[i];
```

이 한 줄이 CPU 레벨에서는 **3단계 비원자적 연산(Atomic Operation)**:
1. LOAD: `global_sum` 읽기 → 레지스터
2. ADD: 더하기
3. STORE: 결과 쓰기

**두 스레드(Thread) 동시 실행 시**:
```
Thread A              global_sum     Thread B
   read → 100           100            read → 100
   add  → 110                          add  → 105
   write → 110          110            write → 105
                        ↑
                     최종 105 (A의 +10 손실!)
```

### 4.11.2 해결 방법

**방법 1 — 별도 저장소 (가장 간단)**:
```c
int partial_sum[NUM_THREADS];

void *work(void *arg) {
    int id = *(int *)arg;
    partial_sum[id] = 0;
    for (int i = start; i < end; i++)
        partial_sum[id] += array[i];  // 충돌 없음!
    return NULL;
}
```

**방법 2 — 뮤텍스(Mutex) (9주차에서 상세)**:
```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_lock(&mutex);
global_sum += value;
pthread_mutex_unlock(&mutex);
```

## 4.12 Java 스레드(Thread)

### 4.12.1 두 가지 생성 방법

```java
// 방법 1: Thread 상속 (비권장)
class Task extends Thread {
    public void run() { /* ... */ }
}
new Task().start();

// 방법 2: Runnable 구현 (권장)
class Task implements Runnable {
    public void run() { /* ... */ }
}
new Thread(new Task()).start();

// 람다 (Java 8+)
new Thread(() -> { /* ... */ }).start();
```

**Runnable 권장 이유**: Java 는 다중 상속 불가 → 다른 클래스 상속과 호환.

### 4.12.2 start() vs run() ⚠️

```c
task.run();   // ❌ 같은 스레드에서 실행 (새 스레드 생성 안 됨!)
task.start(); // ✅ 새 스레드 생성 후 run() 실행
```

### 4.12.3 join()

```java
Thread worker = new Thread(new Task());
worker.start();
try {
    worker.join();  // 완료 대기
} catch (InterruptedException ie) { }
```

---

## 4장 정리 🎯

**4장의 큰 그림**: 스레드(Thread)는 프로세스(Process) 내 가벼운 실행 단위로 자원을 공유한다. 멀티스레딩은 응답성(Responsiveness)·자원 공유(Resource Sharing)·경제성(Economy)·확장성(Scalability)의 4 이점을 주지만 경쟁 조건(Race Condition)을 초래한다. 일대일(One-to-One) 모델이 현대 표준. 암달의 법칙(Amdahl's Law)은 병렬화의 절대 한계.

---

# 제5장. 암묵적 스레딩(Implicit Threading)과 스레딩 이슈 (W05)

## 5.1 암묵적 스레딩(Implicit Threading) — 왜 필요한가

명시적 스레딩(Explicit Threading)(`pthread_create`) 은 수백 개의 스레드(Thread)를 직접 관리해야 해서 비현실적. 해결: **태스크만 정의** 하고 **스케줄링은 라이브러리에 위임**.

**5가지 대표 기법 — T-F-O-G-I**:
- **T**hread Pool
- **F**ork-Join
- **O**penMP
- **G**CD (Grand Central Dispatch)
- **I**ntel TBB

## 5.2 스레드 풀 (Thread Pool)

### 5.2.1 개념

미리 **고정 개수의 워커 스레드(Thread)** 생성. 태스크를 **큐에 제출** 하면 워커가 꺼내 실행. 완료 후 워커는 풀로 반환되어 재사용.

### 5.2.2 세 가지 이점 🎯

1. **속도** — 재사용으로 생성 비용 제거
2. **자원 제한** — 동시 스레드(Thread) 수 상한 (시스템 보호)
3. **분리** — 태스크 정의와 실행 전략 분리 (지연 실행, 주기(Period) 실행 등 유연)

### 5.2.3 Java ExecutorService

```java
ExecutorService pool = Executors.newFixedThreadPool(4);
for (int i = 0; i < 100; i++)
    pool.execute(new Task(i));
pool.shutdown();
```

**팩토리 메서드**:
| 메서드 | 용도 |
|---|---|
| `newSingleThreadExecutor()` | 크기 1, 순차 |
| `newFixedThreadPool(n)` | 고정 n (**서버 표준**) |
| `newCachedThreadPool()` | 동적 조정 |

### 5.2.4 Callable/Future — 결과 반환

`Runnable` 은 반환값 없음(void). 결과 필요 시 `Callable<V>`.

```java
class Summation implements Callable<Integer> {
    private int upper;
    public Summation(int u) { upper = u; }
    public Integer call() {
        int sum = 0;
        for (int i = 1; i <= upper; i++) sum += i;
        return sum;
    }
}

// 사용
Future<Integer> result = pool.submit(new Summation(10));
int answer = result.get();  // 결과 준비까지 블록
pool.shutdown();
```

## 5.3 Fork-Join

### 5.3.1 패턴

재귀적 **분할 정복(Divide and Conquer)** 에 이상적.

```
fork ─┬→ Task 1
      ├→ Task 2
      └→ Task 3
         ↓
       join
         ↓
      결합
```

### 5.3.2 Java 예제 — 배열 합산

```java
class SumTask extends RecursiveTask<Integer> {
    static final int THRESHOLD = 1000;
    private int begin, end;
    private int[] array;

    public SumTask(int b, int e, int[] a) {
        begin = b; end = e; array = a;
    }

    protected Integer compute() {
        if (end - begin < THRESHOLD) {
            int sum = 0;
            for (int i = begin; i <= end; i++) sum += array[i];
            return sum;
        } else {
            int mid = (begin + end) / 2;
            SumTask left = new SumTask(begin, mid, array);
            SumTask right = new SumTask(mid+1, end, array);
            left.fork(); right.fork();
            return left.join() + right.join();
        }
    }
}
```

### 5.3.3 작업 훔치기 (Work Stealing)

각 스레드(Thread)가 자기 **deque** 유지. 큐가 비면 다른 스레드의 **반대쪽 끝** 에서 태스크 훔침.

- **분산 부하 분산(Load Balancing)** — 중앙 스케줄러 없이 자체 조율
- **캐시 친화적** — 소유자는 LIFO (지역성), 도둑은 FIFO
- **경합 최소** — deque 의 양쪽 끝 사용

## 5.4 OpenMP

### 5.4.1 기본 사용

```c
#include <omp.h>

#pragma omp parallel
{
    int tid = omp_get_thread_num();
    printf("Thread %d\n", tid);
}
```

컴파일: `gcc -fopenmp prog.c`

### 5.4.2 parallel for

```c
#pragma omp parallel for
for (int i = 0; i < N; i++) {
    c[i] = a[i] + b[i];
}
```

- 반복 공간을 **스레드(Thread) 수로 분할**
- **암묵적 배리어**: 모든 스레드가 끝나야 다음 진행

### 5.4.3 Reduction

```c
int sum = 0;
#pragma omp parallel for reduction(+:sum)
for (int i = 0; i < N; i++) sum += arr[i];
```

**내부**: 각 스레드(Thread)가 **로컬 복사본** 에 누적 → 최종적으로 안전하게 결합.

**지원 연산자**: `+`, `-`, `*`, `/`, `&`, `|`, `^`, `&&`, `||`, `max`, `min`

### 5.4.4 데이터 공유 절

- `shared(var)` — 모든 스레드(Thread)가 공유 (기본)
- `private(var)` — 각 스레드 복사본
- `reduction(op:var)` — 안전한 결합

### 5.4.5 주요 함수

| 함수 | 용도 |
|---|---|
| `omp_get_thread_num()` | 현재 스레드(Thread) ID |
| `omp_get_num_threads()` | 전체 스레드 수 |
| `omp_set_num_threads(n)` | 스레드 수 설정 |
| `omp_get_wtime()` | 벽시계 시간 (벤치마크) |

환경변수: `OMP_NUM_THREADS=4 ./prog`

## 5.5 GCD (Grand Central Dispatch)

Apple 이 macOS/iOS 용으로 개발. **디스패치 큐** 에 블록/클로저 제출.

### 5.5.1 큐 유형

- **직렬 큐**: FIFO, 한 번에 하나
- **동시 큐**: FIFO 디큐, 여러 태스크 동시 실행. QoS 클래스로 우선순위.

### 5.5.2 QoS 클래스

| 클래스 | 용도 | 우선 |
|---|---|---|
| `USER_INTERACTIVE` | UI, 즉시 응답 | 최고 |
| `USER_INITIATED` | 사용자 대기 중 | 높음 |
| `UTILITY` | 장시간 BG | 중간 |
| `BACKGROUND` | 인덱싱·백업 | 낮음 |

### 5.5.3 사용 예

```c
dispatch_queue_t queue = dispatch_get_global_queue(
    DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
dispatch_async(queue, ^{ printf("Async\n"); });
```

## 5.6 Intel TBB

C++ 템플릿 라이브러리. 특별한 컴파일러/언어 확장 불필요.

```cpp
#include <tbb/tbb.h>
parallel_for(size_t(0), n, [=](size_t i) { apply(v[i]); });

// Reduce
int sum = parallel_reduce(
    blocked_range<int>(0, n), 0,
    [&](const blocked_range<int>& r, int s) {
        for (int i = r.begin(); i < r.end(); i++) s += data[i];
        return s;
    },
    [](int x, int y) { return x + y; }
);
```

## 5.7 5가지 기법 비교

| 기법 | 언어 | 핵심 | 적합 |
|---|---|---|---|
| Thread Pool | Java, C# | 재사용 | 범용 서버 |
| Fork-Join | Java | 분할 정복(Divide and Conquer) | 재귀 병렬 |
| OpenMP | C/C++/Fortran | 지시어 | HPC |
| GCD | macOS/iOS | 큐 + QoS | 모바일 UI |
| Intel TBB | C++ | 람다 + 캐시 인지 | 고성능 C++ |

## 5.8 스레딩 이슈 1 — fork() / exec() 의미론

### 5.8.1 문제

멀티스레드(Thread) 프로세스(Process)의 한 스레드가 fork() 호출 시:
- **옵션 A**: 모든 스레드 복제
- **옵션 B**: 호출 스레드만 복제

**POSIX 기본**: 호출 스레드만 복제.

### 5.8.2 고아 락 (Orphaned Lock) 위험 ⚠️

```c
// 부모 프로세스
Thread 2: pthread_mutex_lock(&mutex);  // 뮤텍스 획득
          // ...작업 중...

Thread 3: fork();  // Thread 3만 자식에 복제됨

// 자식 프로세스
자식: pthread_mutex_lock(&mutex);
      → DEADLOCK! (잠긴 뮤텍스, 해제할 스레드 없음)
```

### 5.8.3 해결: fork + exec

```c
pid_t pid = fork();
if (pid == 0) {
    execlp("echo", "echo", "Hello", NULL);  // 즉시 exec
    perror("exec"); exit(1);
}
```

`exec()` 가 프로세스(Process) 이미지 전체를 교체하므로 상속된 뮤텍스(Mutex) 문제가 자동 해결됩니다.

## 5.9 스레딩 이슈 2 — 시그널(Signal) 처리

### 5.9.1 동기 vs 비동기 시그널(Asynchronous Signal)

- **동기 (SIGSEGV, SIGFPE)**: 프로세스(Process) 자신의 동작 → 유발 스레드(Thread)에 전달
- **비동기 (SIGTERM, SIGALRM)**: 외부 출처 → **어느 스레드에 전달할지 문제**

### 5.9.2 4가지 전달 옵션

1. 유발 스레드(Thread)에 (동기 시그널(Synchronous Signal))
2. 모든 스레드에 (브로드캐스트)
3. 차단하지 않은 스레드에 (POSIX 기본)
4. **특정 전담 스레드에** (권장)

### 5.9.3 권장 패턴 — 전담 스레드(Thread)

```c
void *signal_handler_thread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int sig;
    while (1) {
        sigwait(set, &sig);  // 블로킹
        switch (sig) {
            case SIGTERM: exit(0);
            case SIGINT: /* 처리 */ break;
        }
    }
}

int main() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);

    pthread_sigmask(SIG_BLOCK, &set, NULL);  // 모든 스레드 차단
    pthread_t handler;
    pthread_create(&handler, NULL, signal_handler_thread, &set);
    // ...
}
```

## 5.10 스레딩 이슈 3 — 스레드(Thread) 취소

### 5.10.1 두 가지 모드

| | 비동기 취소(Asynchronous Cancellation) | 지연 취소(Deferred Cancellation) (기본) |
|---|---|---|
| 시점 | 즉시 | 취소 지점(Cancellation Point)에서만 |
| 안전 | **위험** (자원 누수) | 안전 |

### 5.10.2 취소 지점(Cancellation Point)

POSIX 의 취소 지점(Cancellation Point):
- 블로킹 시스템 콜(System Call): `read`, `write`, `open`, `sleep`, `pause`
- 동기화: `pthread_join`, `pthread_mutex_lock`, `pthread_cond_wait`
- 명시: `pthread_testcancel()`

### 5.10.3 정리 핸들러

```c
void cleanup_fn(void *arg) {
    free(arg);
    printf("Cleanup\n");
}

void *worker(void *arg) {
    char *buf = malloc(1024);
    pthread_cleanup_push(cleanup_fn, buf);

    while (1) {
        sleep(1);
        pthread_testcancel();
    }

    pthread_cleanup_pop(1);  // 1 = 정리 실행
    return NULL;
}
```

### 5.10.4 Java 는 인터럽트(Interrupt)

Java 는 비동기 취소(Asynchronous Cancellation) 제공 X, **인터럽트(Interrupt)** 사용.
```java
Thread worker = new Thread(() -> {
    while (!Thread.currentThread().isInterrupted()) {
        try { Thread.sleep(100); }
        catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            break;
        }
    }
});
worker.start();
worker.interrupt();
```

## 5.11 스레딩 이슈 4 — TLS (스레드-로컬 저장소(Thread-Local Storage, TLS))

### 5.11.1 왜 필요한가

**문제**: `errno` — Thread A 가 `open()` 실패해 `errno=ENOENT` 설정, Thread B 가 성공해 0으로 덮어쓰면 A 가 잘못된 오류 정보를 봄.

**해결**: 각 스레드(Thread)의 독립 복사본 = TLS.

### 5.11.2 지역 변수와의 차이

| | 지역 변수 | TLS |
|---|---|---|
| 선언 | 함수 내부 | 전역 스토리지 |
| 수명 | 함수 호출 중만 | **스레드(Thread) 전체 수명** |
| 가시성 | 함수 범위 | **모든 함수** |
| 저장 | 스택 | 스레드별 특수 영역 |

### 5.11.3 언어별 구현

```c
// C (GCC)
__thread int threadID;

// C11 표준
_Thread_local int var;

// Pthreads API
pthread_key_t key;
pthread_key_create(&key, free);
pthread_setspecific(key, value);
void *v = pthread_getspecific(key);
```

```java
// Java
ThreadLocal<Integer> threadID = ThreadLocal.withInitial(() -> 0);
threadID.set(42);
int id = threadID.get();
```

### 5.11.4 실습 예제

```c
int shared_var = 0;        // 경쟁 조건!
__thread int tls_var = 0;  // 안전

void *worker(void *arg) {
    for (int i = 0; i < 100000; i++) {
        shared_var++;   // 경쟁 — 결과 부정확
        tls_var++;      // 안전 — 각 스레드 독립
    }
    // tls_var 는 항상 100000 (정확)
    // shared_var 는 불확정
}
```

## 5.12 스레딩 이슈 5 — LWP와 스케줄러 활성화(Scheduler Activation)

### 5.12.1 LWP (Lightweight Process)

사용자 스레드(User Thread)와 커널 스레드(Kernel Thread) 사이의 **중간 계층**. 라이브러리에 **가상 CPU** 로 보임.

```
User Thread
     ↓
    LWP (가상 프로세서)
     ↓
 Kernel Thread
     ↓
 물리 CPU
```

### 5.12.2 업콜 (Upcall)

다대다(Many-to-Many) 모델에서 커널(Kernel)과 라이브러리 간 **양방향 통신**.

```
시나리오: Thread A 가 read() 에서 블록
1. 커널 "Thread A 블로킹" → 업콜 발생
2. 라이브러리 업콜 핸들러: 다른 스레드를 LWP 에 할당
3. I/O 완료 → 커널이 다시 업콜: "A 준비됨"
```

🎯 **복잡성**: 구현 난이도가 매우 높아 현대 OS 는 대부분 **일대일(One-to-One)** 로 회귀.

## 5.13 Windows 스레드(Thread) 구조

Windows 는 **일대일(One-to-One) 모델**.

| 구조 | 위치 | 내용 |
|---|---|---|
| **ETHREAD** | 커널(Kernel) | 시작 주소, 부모, KTHREAD 포인터 |
| **KTHREAD** | 커널 | 상태, 우선순위, 커널 스택 |
| **TEB** | 유저 | TID, 유저 스택, TLS 배열 |

## 5.14 Linux 의 통합 모델 — clone()

### 5.14.1 혁신적 설계

Linux 는 **프로세스(Process)와 스레드(Thread)를 구분 안 함**. 모두 **태스크(task)** 로, 공통 `task_struct` 사용. 공유 수준은 `clone()` 플래그로 제어.

### 5.14.2 clone 플래그

| 플래그 | 공유 |
|---|---|
| `CLONE_VM` | 가상 메모리(Virtual Memory) |
| `CLONE_FS` | 파일시스템 정보 |
| `CLONE_FILES` | fd 테이블 |
| `CLONE_SIGHAND` | 시그널(Signal) 핸들러 |
| `CLONE_THREAD` | 스레드(Thread) 그룹 |
| `CLONE_NEWNS/PID/NET` | 네임스페이스(Namespace) (컨테이너(Container)) |

### 5.14.3 세 가지 케이스

- **fork()** = `clone(0)` — 아무것도 공유 X
- **pthread_create** ≈ `clone(CLONE_VM|FS|FILES|SIGHAND|THREAD)` — 거의 전부 공유
- **컨테이너(Container)** = `clone(..|CLONE_NEWNS|NEWPID|NEWNET)` — 부분 격리

### 5.14.4 컨테이너(Container) 구현

Docker, LXC 가 이 모델 사용:
```c
clone(container_init, stack,
    CLONE_VM
    | CLONE_NEWNS     // 파일시스템 격리
    | CLONE_NEWPID    // 프로세스 트리 격리
    | CLONE_NEWNET,   // 네트워크 격리
    NULL);
```

🎯 **결과**: "경량 가상머신" 같은 컨테이너(Container)가 자연스럽게 구현됩니다.

---

## 5장 정리 🎯

**5장의 큰 그림**: 개발자는 태스크만 정의하고 라이브러리가 스레드(Thread) 관리(암묵적 스레딩(Implicit Threading)). 하지만 스레드는 5가지 까다로운 이슈(fork/exec, 시그널(Signal), 취소, TLS, LWP)를 안고 있다. Linux 의 clone()은 프로세스(Process)/스레드를 통합해 컨테이너(Container)까지 자연스럽게 지원.

---

# 제6장. CPU 스케줄링 기본 알고리즘 (W06)

## 6.1 스케줄링의 전제 — CPU-I/O 버스트(I/O Burst) 사이클

### 6.1.1 프로세스(Process) 실행 패턴

프로세스(Process) 실행은 **CPU 버스트(CPU Burst) ↔ I/O 버스트(I/O Burst) 의 반복** 입니다.

```
CPU burst → I/O burst → CPU burst → I/O burst → ... → exit
 (계산)    (디스크·네트워크)  (계산)   (I/O)
```

### 6.1.2 두 유형의 프로세스(Process)

| 유형 | 특성 | 예시 |
|---|---|---|
| **I/O 바운드(I/O Bound)** | 짧은 CPU 버스트(CPU Burst) 많음 | 브라우저, DB |
| **CPU 바운드(CPU Bound)** | 긴 CPU 버스트 적음 | 과학 계산, 렌더링 |

혼합 시 CPU 와 장치 모두 효율적.

## 6.2 스케줄링 결정 시점과 선점형(Preemptive)

### 6.2.1 네 가지 결정 시점

1. Running → Waiting (자발적, 비선점(Non-preemptive))
2. **Running → Ready (선점)**
3. **Waiting → Ready (선점 가능)**
4. 종료

### 6.2.2 선점형(Preemptive) vs 비선점형(Non-preemptive)

- **비선점형(Non-preemptive)**: 1·4 에서만 스케줄링 — 프로세스(Process)가 자발적으로 양보. 단순.
- **선점형(Preemptive)**: 2·3 에서도 결정 — OS 가 강제로 CPU 회수. 응답성(Responsiveness) 좋지만 공유 데이터 보호 필요. **현대 OS 표준**.

## 6.3 디스패처 (Dispatcher)

**정의**: 스케줄러가 선택한 프로세스(Process)로 **실제 제어권을 이전** 하는 모듈.

**3가지 역할**:
1. **컨텍스트 스위칭(Context Switching)** — 레지스터 저장/복원
2. **유저/커널 모드(Kernel Mode) 전환**
3. **PC 점프** — 사용자 프로그램의 적절한 위치로

**디스패치 지연 (Dispatch Latency)**: 한 프로세스 정지 → 다른 프로세스 시작까지 시간. 일반적으로 1~10 µs.

🔗 **연결**: 이 지연이 라운드 로빈(Round Robin, RR) 퀀텀 선택의 하한선을 결정합니다 (6.7 참조).

## 6.4 스케줄링 기준 🎯

다섯 가지 평가 지표:

| 기준 | 의미 | 방향 |
|---|---|---|
| **CPU 이용률(CPU Utilization)** | CPU 가 유휴가 아닌 비율 | 최대 |
| **처리량(Throughput)** | 단위 시간당 완료 프로세스(Process) | 최대 |
| **반환 시간(Turnaround Time)** | 제출 ~ 완료 | 최소 |
| **대기 시간(Waiting Time)** | **준비 큐(Ready Queue) 체류 시간** | **최소 (핵심)** |
| **응답 시간(Response Time)** | 제출 ~ 첫 응답 | 최소 (대화형) |

**반환 시간의 구성**:
```
반환 시간 = 대기 시간 + CPU 버스트 시간 + I/O 시간
```

🎯 **왜 대기 시간이 핵심 지표인가**: CPU 버스트(CPU Burst)와 I/O 시간은 프로세스 자체 속성 — 스케줄러가 바꿀 수 없음. 스케줄링이 영향 주는 것은 **오직 대기 시간** 입니다.

## 6.5 FCFS (First-Come, First-Served)

### 6.5.1 규칙

도착 순서대로 실행. 비선점(Non-preemptive).

### 6.5.2 예시 계산 🧪

**프로세스(Process)**: P1(24), P2(3), P3(3) — 도착 순서대로 가정

**간트 차트 (P1-P2-P3 순서)**:
```
| P1         | P2 | P3 |
0           24   27   30
```

**대기 시간(Waiting Time)**:
- P1: 0
- P2: 24
- P3: 27
- **평균: (0+24+27)/3 = 17**

**순서가 P2-P3-P1 이면?**:
```
| P2 | P3 | P1         |
0    3    6           30
```
- 평균 대기: (0+3+6)/3 = **3**

### 6.5.3 호위 효과 (Convoy Effect) 🎯

**문제**: 긴 프로세스(Process)가 앞에 있으면 뒤의 짧은 프로세스들이 모두 대기 → 평균 대기 폭증.

FCFS 는 **도착 순서에 극도로 민감** — 같은 프로세스 집합이라도 순서에 따라 17 vs 3 처럼 극단적 차이.

### 6.5.4 특징 요약

- **장점**: 단순, 구현 쉬움
- **단점**: 호위 효과(Convoy Effect), 성능 변동성 큼
- **응답 시간(Response Time)**: 나쁨

## 6.6 SJF (Shortest Job First)

### 6.6.1 규칙

다음 CPU 버스트(CPU Burst)가 **가장 짧은** 프로세스(Process) 우선. 비선점(Non-preemptive).

### 6.6.2 왜 최적인가

**이론적으로 평균 대기 시간(Waiting Time)을 최소화** 한다는 것이 증명됨.

💡 **직관**: 짧은 작업을 먼저 처리하면, 뒤의 프로세스(Process)들이 기다리는 **총 시간** 이 최소화됨. 긴 작업은 어차피 오래 걸리는데 뒤로 미뤄도 자기 대기 시간만 늘고 여러 짧은 작업의 대기 시간은 줄어든다.

### 6.6.3 예시 계산 🧪

**프로세스(Process)와 버스트**: P1(6), P2(8), P3(7), P4(3)

**SJF 간트 차트(Gantt Chart) (모두 t=0 도착 가정)**:
```
| P4 | P1    | P3     | P2      |
0    3       9       16        24
```

**대기 시간(Waiting Time)**:
- P4: 0, P1: 3, P3: 9, P2: 16
- **평균: (0+3+9+16)/4 = 7**

**FCFS 로 같은 프로세스 (P1-P2-P3-P4 순서)**:
```
| P1    | P2      | P3     | P4 |
0        6       14      21   24
```
- 평균: (0+6+14+21)/4 = **10.25**

### 6.6.4 현실의 장벽 — 예측 필요 ⚠️

**문제**: 실제로는 다음 CPU 버스트(CPU Burst) 길이를 **알 수 없음**.

**해결**: **지수 평균(Exponential Averaging)** 으로 과거에서 예측.

$$ \tau_{n+1} = \alpha \cdot t_n + (1-\alpha) \cdot \tau_n $$

- $t_n$ — n 번째 실제 버스트 길이
- $\tau_{n+1}$ — (n+1)번째 예측값
- $\alpha$ — 최근 값 가중치 (0 ≤ α ≤ 1)

**α 값 해석**:
- α = 0: 과거 예측만 사용 (변화 반영 X)
- α = 1: 가장 최근 값만 (과거 무시)
- α = 0.5 (보통): 균형

### 6.6.5 기아(Starvation) 문제

**기아(Starvation)**: 긴 프로세스(Process)가 계속 짧은 것들에 밀려 영원히 실행 못 함.

**해결**: **노화(Aging)** — 대기 시간(Waiting Time)이 길어질수록 우선순위 상승.

## 6.7 SRTF (Shortest Remaining Time First) — SJF 선점형(Preemptive)

### 6.7.1 규칙

새로 도착한 프로세스(Process)의 버스트가 현재 실행 중 프로세스의 **남은 시간** 보다 짧으면 **선점(Preemption)**.

### 6.7.2 예시 계산 🧪

**프로세스(Process)** (도착 시각, 버스트):
- P1 (0, 8), P2 (1, 4), P3 (2, 9), P4 (3, 5)

**SRTF 간트**:
```
t=0: P1 도착, 실행 시작 (8 남음)
t=1: P2 도착 (4 < 7 남은 P1). 선점! P2 실행
t=2: P3 도착 (9). P2 남은 3 < 9. 계속 P2
t=3: P4 도착 (5). P2 남은 2 < 5. 계속 P2
t=5: P2 완료. 후보 — P1(7 남음), P3(9), P4(5). P4 선택
t=10: P4 완료. 후보 — P1(7), P3(9). P1
t=17: P1 완료. P3 실행
t=26: P3 완료
```

```
| P1 | P2     | P4     | P1      | P3       |
0    1        5       10        17        26
```

**대기 시간(Waiting Time) 계산**:
- P1: (0에서 시작, 1-5 선점(Preemption), 10-17 실행) → 대기 = (1-0) + (10-5) = 9? 
  - 실제로 대기 = 10-1 = 9 (t=0~1 실행, t=1~10 대기, t=10~17 실행)
- P2: 0 (t=1 도착, t=1 시작)
- P3: 17-2 = 15
- P4: 5-3 = 2
- **평균: (9+0+15+2)/4 = 6.5**

### 6.7.3 특징

- SJF 보다 일반적으로 **평균 대기 시간(Waiting Time) 더 낮음**
- **기아(Starvation) 위험 더 큼** (긴 작업이 더 자주 밀림)

## 6.8 라운드 로빈 (RR)

### 6.8.1 규칙

각 프로세스(Process)에 **타임 퀀텀(Time Quantum) q** 할당. 퀀텀 만료 시 **선점(Preemption)** 후 큐 뒤로.

### 6.8.2 핵심 특성

- **기아(Starvation) 없음** — N 개 프로세스(Process)에서 한 프로세스 최대 대기 = (N-1) × q
- **응답 시간(Response Time) 우수** — 대화형에 최적

### 6.8.3 예시 계산 🧪

**프로세스(Process)**: P1(24), P2(3), P3(3), 모두 t=0 도착, q = 4

**간트 차트(Gantt Chart)**:
```
| P1 | P2 | P3 | P1 | P1 | P1 | P1 | P1 |
0    4    7   10   14   18   22   26   30
```

- P1 은 4씩 7번 실행 (24 = 4×6)... 실제: 4, 4, 4, 4, 4, 4 = 총 24
- 실제 순서: P1(0-4) → P2(4-7) → P3(7-10) → P1(10-14) → P1(14-18) → P1(18-22) → P1(22-26) → P1(26-30)

**대기 시간(Waiting Time)**:
- P1: 완료 시각 30, 대기 = 30 - 24 = 6
- P2: 완료 7, 대기 = 7 - 3 = 4
- P3: 완료 10, 대기 = 10 - 3 = 7
- **평균: (6+4+7)/3 = 5.67**

### 6.8.4 타임 퀀텀(Time Quantum) 선택 🎯

**딜레마**:
- **너무 크면** → FCFS 로 수렴 (응답 시간(Response Time) 악화)
- **너무 작으면** → 컨텍스트 스위칭(Context Switching) 오버헤드 지배

**80% 규칙**: CPU 버스트(CPU Burst)의 **80%가 한 퀀텀 내 완료** 되도록 q 설정.

**수치 예시**:
- 디스패치 지연(Dispatch Latency) 10 µs, q = 100 µs 면 오버헤드 약 10%
- q = 1 ms 면 오버헤드 1%
- q = 10 ms 면 오버헤드 0.1%, 대신 응답 시간 10ms 까지 증가 가능

### 6.8.5 특징

- **장점**: 공정, 기아(Starvation) 없음, 응답 시간(Response Time) 좋음
- **단점**: 평균 대기는 SJF 보다 나쁠 수 있음, 퀀텀 선택 중요

## 6.9 우선순위 스케줄링(Priority Scheduling)

### 6.9.1 규칙

각 프로세스(Process)에 **우선순위 숫자**. 최고 우선순위 먼저.

- 일반적으로 **작은 숫자 = 높은 우선순위**
- 선점(Preemption)/비선점 모두 가능
- **SJF = 우선순위 = 1/(다음 버스트)** 의 특수 경우

### 6.9.2 기아(Starvation) → 노화(Aging) 해결

**노화(Aging)**: 대기 시간(Waiting Time)이 길수록 우선순위 ↑.
예: 매 1분마다 우선순위 1씩 상승 → 결국 모든 프로세스(Process)가 실행 기회를 얻음.

## 6.10 네 알고리즘 종합 비교 🎯

| | 유형 | 평균 대기 | 응답 | 기아(Starvation) | 장점 | 단점 |
|---|---|---|---|---|---|---|
| **FCFS** | 비선점(Non-preemptive) | 큼 | 나쁨 | X | 단순 | **호위 효과(Convoy Effect)** |
| **SJF** | 비선점 | **최소** | 보통 | O | 최적 | 예측 필요 |
| **SRTF** | 선점 | SJF 보다 ↓ | 좋음 | O | 더 좋음 | 오버헤드 |
| **RR** | 선점 | 큼 | **매우 좋음** | X | **공정, 응답성(Responsiveness)** | 퀀텀 선택 |
| **Priority** | 양쪽 | 가변 | 가변 | O→노화(Aging) | 중요도 반영 | 기아 위험 |

## 6.11 xv6 컨텍스트 스위칭(Context Switching) — struct context 와 swtch.S

### 6.11.1 struct context

```c
struct context {
    uint64 ra;     // 반환 주소 (PC)
    uint64 sp;     // 스택 포인터
    uint64 s0;     // 피호출자 저장 레지스터
    uint64 s1;
    // ...
    uint64 s11;
};
```

**왜 피호출자 저장 레지스터만?**: 호출자 저장 레지스터(t0~t6, a0~a7)는 C 컴파일러가 이미 스택에 저장함. swtch() 를 부르는 C 코드가 돌아오는 걸 가정하기 때문.

### 6.11.2 swtch.S 흐름

```assembly
swtch:
    # old.context (*a0) 에 현재 레지스터 저장
    sd ra, 0(a0)
    sd sp, 8(a0)
    sd s0, 16(a0)
    # ... s1~s11 ...

    # new.context (*a1) 에서 레지스터 복원
    ld ra, 0(a1)
    ld sp, 8(a1)
    ld s0, 16(a1)
    # ... s1~s11 ...

    ret  # ra 가 새 프로세스의 PC 로 점프
```

🎯 **핵심**: `ret` 이 실행되면 **다른 프로세스(Process)의 PC 로 점프** 됩니다. swtch() 를 호출한 코드가 **다른 프로세스의 맥락에서** 돌아옵니다.

---

## 6장 정리 🎯

**6장의 큰 그림**: 스케줄링은 5가지 기준(CPU 이용률(CPU Utilization), 처리량(Throughput), 반환·대기·응답 시간(Response Time)) 중 대기 시간(Waiting Time)이 핵심이다. FCFS(단순하나 호위 효과(Convoy Effect)), SJF(최적이나 예측 필요), SRTF(SJF 선점형(Preemptive)), RR(공정·응답성(Responsiveness)). 모든 알고리즘은 트레이드오프의 타협.

---

# 제7장. 고급 스케줄링 — 멀티프로세서와 실시간 (W07)

## 7.1 다단계 큐 (MLQ, Multilevel Queue)

### 7.1.1 개념

프로세스(Process)를 유형별로 **별도 큐**에 배치. 큐마다 **독립 스케줄링** 정책.

```
| Foreground Queue | → RR (짧은 퀀텀)
| Background Queue | → FCFS

큐 간: 고정 우선순위 (Foreground 우선)
```

### 7.1.2 한계

**정적 분류**: 프로세스(Process)가 **영구적으로** 한 큐에 배정 → 행동 변화 반영 못 함.

## 7.2 다단계 피드백 큐 (MLFQ) 🎯

### 7.2.1 세 가지 규칙

1. **새 프로세스(Process)**: 최상위 큐 (Q0) 에 진입
2. **강등(Demotion)**: 큐의 퀀텀 소진 → 다음 하위 큐
3. **승격(Promotion, Aging)**: 오래 대기 시 → 상위 큐

### 7.2.2 예시 구성

| 큐 | 스케줄링 | 퀀텀 | 역할 |
|---|---|---|---|
| Q0 | RR | 8 ms | 짧은/인터랙티브 |
| Q1 | RR | 16 ms | 중간 작업 |
| Q2 | FCFS | 무제한 | 긴 CPU-bound |

### 7.2.3 왜 천재적인가 🎯

프로세스(Process)가 **어느 유형인지 미리 알 필요 없음**. 행동 관찰로 **자동 분류**:
- 짧은 작업 → Q0 에서 완료 (낮은 반환 시간(Turnaround Time))
- I/O 바운드(I/O Bound) → I/O 시 자발적 양보 → Q0 유지 (인터랙티브 유리)
- CPU 바운드(CPU Bound) → 퀀텀 계속 소진 → Q2 로 강등(Demotion)
- 오래 기다린 Q2 프로세스 → 승격(Promotion) (기아(Starvation) 방지)

🎯 **결과**: **SJF 를 근사** 하면서도 **기아 방지**. 현대 UNIX 계열 기본 전략.

### 7.2.4 현대 OS 사례

- **Linux O(1) 스케줄러** (2.6 이전): 140 레벨 MLFQ
- **Solaris Time-Sharing Class**
- **Windows**: 멀티레벨 큐 + 동적 부스트

## 7.3 스레드(Thread) 스케줄링 — PCS vs SCS

### 7.3.1 두 가지 경쟁 범위

| | PCS (Process-Contention Scope) | SCS (System-Contention Scope) |
|---|---|---|
| 경쟁 | 프로세스(Process) 내 스레드(Thread)끼리 | **시스템 전체 스레드** |
| 사용 모델 | 다대일(Many-to-One), 다대다(Many-to-Many) | **일대일(One-to-One)** |
| 현재 | 제한적 | **현대 표준** |

### 7.3.2 Pthreads scheduling contention scope

```c
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);  // SCS
// 또는 PTHREAD_SCOPE_PROCESS
```

Linux, macOS 는 **SCS 만 지원** (일대일(One-to-One) 모델이므로).

## 7.4 멀티프로세서 스케줄링

### 7.4.1 SMP (Symmetric Multiprocessing)

각 CPU 가 **독립적으로** 스케줄링 결정.
- **대칭** — 모든 CPU 가 같은 역할
- **공통 준비 큐(Ready Queue)** 또는 **CPU별 큐** 가능

**비대칭(Asymmetric)**: 마스터 CPU 만 결정, 나머지는 사용자 코드만. 단순하나 병목.

### 7.4.2 프로세서 친화도 (Affinity)

**개념**: 프로세스(Process)를 **같은 CPU 에 유지** 하려는 경향.

**왜 필요**:
- 프로세스가 CPU A 에서 실행되면 A 의 캐시에 데이터 로드됨
- CPU B 로 이동하면 B 의 캐시는 비어 있음 → **캐시 미스 폭증 → 성능 ↓**

| | 설명 |
|---|---|
| **Soft Affinity** | 가능하면 같은 CPU (대부분 OS) |
| **Hard Affinity** | 특정 CPU 에 강제 고정 (`sched_setaffinity()`) |

### 7.4.3 NUMA (Non-Uniform Memory Access)

**문제**: 멀티소켓(Socket) 시스템에서 코어마다 **메모리 지연이 다름**.
- CPU 0 → 메모리 A: 100 ns (로컬)
- CPU 0 → 메모리 B: 200 ns (원격)

**스케줄링 결정**: 프로세스(Process)를 **메모리가 있는 근처 CPU** 에 배치 (NUMA-aware).

### 7.4.4 부하 분산 (Load Balancing)

- **Push Migration**: 과부하 CPU → 다른 CPU 로 밀어냄 (주기(Period)적 검사)
- **Pull Migration**: 유휴 CPU → 바쁜 CPU 의 큐에서 끌어옴

⚠️ **친화도 vs 부하 분산(Load Balancing) 상충**: 캐시 유지 ← 친화도 / 균등 분배 ← 부하 분산. 좋은 스케줄러는 균형을 잡음.

### 7.4.5 SMT (Simultaneous Multithreading) / 하이퍼스레딩(Hyperthreading)

물리 코어 1개에 **여러 하드웨어 스레드(Thread)**. 메모리 스톨 중에 다른 스레드 실행 → 효율 ↑.

**스케줄링 힌트**: 같은 물리 코어의 두 번째 HW 스레드 대신 **다른 물리 코어 먼저** 할당 → 더 나은 병렬성(Parallelism).

## 7.5 실시간 스케줄링(Real-Time Scheduling)

### 7.5.1 Soft vs Hard

| | Soft Real-Time | Hard Real-Time |
|---|---|---|
| 마감(Deadline) 보증 | 최선 노력 | **필수** |
| 위반 영향 | 성능 저하 | **시스템 실패** |
| 예 | 비디오 스트리밍, 게임 | 항공 제어, 의료 기기 |

### 7.5.2 지연 요소

- **Interrupt Latency**: 인터럽트(Interrupt) 도착 → ISR 시작
- **Dispatch Latency**: 실시간 태스크 준비 → 실제 실행
- **Event Latency**: 이벤트 → 서비스 완료 (총합)

### 7.5.3 Rate-Monotonic (RMS)

**정적 우선순위 = 1 / 주기(Period)**. 주기 짧을수록 우선.

🧪 **예시**:
- T1: 주기 50ms, 실행 20ms
- T2: 주기 100ms, 실행 35ms

RMS: T1 우선 (주기 짧음). T1 이 50ms 마다 20ms 실행, T2 는 그 사이 틈에 35ms 완료.

**이용률 한계**:
$$ U \le N(2^{1/N} - 1) $$

| N | 한계 |
|---|---|
| 1 | 1.00 |
| 2 | 0.828 |
| 3 | 0.779 |
| ∞ | **0.693 (약 69.3%)** |

이 이상 이용하면 **마감(Deadline) 놓칠 수 있음**.

### 7.5.4 EDF (Earliest-Deadline-First)

**동적 우선순위 = 가장 가까운 마감(Deadline)**. 우선순위가 매 시점 변함.

**이용률**: 이론상 **100%** 가능.

**조건**: 각 태스크의 마감 정보 필요.

### 7.5.5 Priority Inversion & 상속

**시나리오**:
- T3 (낮은 우선순위) 가 뮤텍스(Mutex) 잠금
- T1 (높은 우선순위) 가 뮤텍스 대기
- T2 (중간 우선순위) 가 T3 를 선점(Preemption)해 실행 → T3 가 뮤텍스 해제 못 함 → **T1 이 T2 에 의해 간접 블록**

**해결: 우선순위 상속(Priority Inheritance) 프로토콜**
- 블로킹하는 T3 의 우선순위를 T1 수준으로 **일시 상승**
- T3 가 뮤텍스 해제 후 원래 우선순위 복귀

🧪 **유명 사례**: 1997년 Mars Pathfinder 탐사선 재부팅 버그. 이 이슈 때문에 발생, 상속 프로토콜 활성화로 해결.

## 7.6 운영체제(Operating System) 사례

### 7.6.1 Linux CFS (Completely Fair Scheduler)

**핵심 개념**:
- **vruntime (virtual run time)**: 각 태스크의 가상 실행 시간(Virtual Run Time, vruntime), 우선순위(nice 값)로 가중
- **RB-Tree**: vruntime 기준 정렬 → O(log N) 에 최소값 선택

**"완전히 공정" 의 의미**:
- 장기적으로 모든 태스크가 **가중치 비율** 대로 CPU 를 받음
- nice 값이 낮을수록 (우선순위 높을수록) vruntime 증가 속도 느림

### 7.6.2 Windows

**32 우선순위 레벨** (0~31). 두 클래스:
- 가변 클래스 (1~15)
- 실시간 클래스 (16~31)

**동적 우선순위 부스트**:
- 키보드/마우스 입력 → 해당 프로세스(Process) 일시 부스트
- 포커스 윈도우 → 부스트
- 결과: **인터랙티브 반응성 극대화**

### 7.6.3 Solaris

우선순위 클래스 6개:
- Time-sharing (TS)
- Interactive (IA)
- Fair share (FSS)
- Fixed priority (FP)
- System (SYS)
- Real-time (RT)

클래스 내부는 MLFQ.

## 7.7 스케줄링 평가 방법 🎯

### 7.7.1 네 가지 방법

| 방법 | 정확도 | 비용 | 한계 |
|---|---|---|---|
| **결정론적 모델링(Deterministic Modeling)** | 명확 | 낮음 | 특정 워크로드만 |
| **큐잉 모델(Queueing Model)** | 일반 | 중간 | 현실적 정책 계산 어려움 |
| **시뮬레이션(Simulation)** | 높음 | 높음 | 구현 비용 |
| **구현** | **완벽** | 매우 높음 | **사용자 행동 변화** |

### 7.7.2 Little's Formula

$$ n = \lambda \cdot W $$

- **n**: 평균 큐 길이
- **λ**: 도착률 (프로세스(Process)/초)
- **W**: 평균 대기 시간(Waiting Time)

🧪 **예시**: λ=7/sec, n=14 → W = 14/7 = **2초**

**일반성**: 어떤 도착 분포, 서비스 시간 분포에도 성립. 유일한 가정은 **정상 상태**.

**한계**: n, λ 계산이 현실적 정책에서는 복잡.

### 7.7.3 시뮬레이션(Simulation)

**방법**: 스케줄러의 소프트웨어 모델 + 합성/트레이스 워크로드.

**워크로드 원천**:
- **합성**: 난수 생성 (편리, 실제와 다를 수 있음)
- **트레이스**: 실제 기록 (정확, 용량 큼)

### 7.7.4 구현 — 유일하게 완전 정확

실제 OS 에 구현하여 사용자에게 배포.

⚠️ **가장 큰 한계**: **사용자 행동이 바뀐다**.

**역사적 예시**: 초기 멀티유저 시스템이 I/O-bound 프로세스(Process)를 자동 부스트 → 시뮬레이션(Simulation)에서 훌륭한 성능. 배포 후 사용자들이 알아채고 CPU-bound 작업을 가짜 I/O(예: `/dev/null` 쓰기) 로 감쌈 → 스케줄러를 "속임" → 평가 무너짐.

🎯 **교훈**: 실제 환경은 **대체 불가능한 평가 방법**.

---

## 7장 정리 🎯

**7장의 큰 그림**: MLFQ 는 SJF 를 근사하면서 기아(Starvation)를 방지하는 천재적 설계. 멀티프로세서에서는 친화도(캐시) vs 부하 분산(Load Balancing)(균등)이 상충. 실시간은 마감(Deadline) 보증이 핵심 — RMS(정적, 69.3% 한계) vs EDF(동적, 100%). 우선순위 역전(Priority Inversion)은 Mars Pathfinder 처럼 치명적이지만 상속 프로토콜로 해결. 평가의 최고 방법은 구현이지만 사용자 행동 변화로 무너질 수 있음.

---

# 제8장. 큰 그림 — 7주간의 개념들이 어떻게 맞물리는가

## 8.1 왜 이 순서로 배우는가

W01 → W07 의 **설계된 학습 곡선**:

```
W01: OS 정의, 이중 모드 (보호)
         ↓ 왜 보호가 필요? 여러 프로그램을 실행하려고
W02: 프로세스 (실행의 추상화)
         ↓ 프로세스는 격리된다. 그런데 협력이 필요
W03: IPC (협력의 방법)
         ↓ IPC 는 무겁다. 같은 주소 공간 공유하는 경량 단위는?
W04: 스레드 (경량 동시 실행)
         ↓ 명시적 스레딩은 복잡. 더 나은 추상화는?
W05: 암묵적 스레딩, 스레딩 이슈
         ↓ 결국 CPU 는 하나 (혹은 몇 개). 어떻게 나눌까?
W06: 기본 스케줄링 알고리즘
         ↓ 현실은 멀티코어, 실시간, 다양한 요구
W07: 고급 스케줄링
```

## 8.2 일곱 가지 핵심 연결 고리

### 🔗 연결 1: 이중 모드(Dual-Mode) → 시스템 콜(System Call) → 모든 OS 기능

OS 가 자신을 보호하기 위해 이중 모드(Dual-Mode)를 만듦. 그 결과 사용자 프로그램은 **시스템 콜(System Call)** 만이 유일한 통로. 그 시스템 콜 안에 `fork`, `exec`, `pipe`, `shm_open`, `pthread_create` 모두 있음. **모든 OS 기능은 이 하나의 보호 메커니즘 위에 세워져 있음**.

### 🔗 연결 2: 프로세스(Process) 격리 ↔ IPC 의 필요성

프로세스(Process)는 안전을 위해 격리되어야 함(W02). 그런데 협력이 필요함(W03). 메시지 전달(Message Passing)은 안전하지만 느리고, 공유 메모리(Shared Memory)는 빠르지만 동기화가 위험함. **이 긴장이 9주차 동기화로 이어짐**.

### 🔗 연결 3: 스레드(Thread)의 양날의 검

스레드(Thread)의 장점(공유 메모리(Shared Memory))이 곧 단점(경쟁 조건(Race Condition)). W04 에서 `partial_sum[id]` 로 회피했고, W05 에서 `reduction`, `ThreadLocal` 로 부분 해결했지만, **근본 해법은 9주차** (세마포어(Semaphore)·뮤텍스(Mutex)·조건 변수(Condition Variable)).

### 🔗 연결 4: 암달의 법칙(Amdahl's Law) → 스케줄링의 정당성

병렬화에는 본질적 한계(S). 코어를 아무리 늘려도 한계가 있으므로 **한정된 CPU 를 효율적으로 나누는 스케줄링** (W06~W07) 이 여전히 중요. "멀티코어가 있으니 스케줄링 걱정 없다" 는 잘못된 직관.

### 🔗 연결 5: PCB → 컨텍스트 스위칭(Context Switching) → 스케줄링 오버헤드

PCB(W02) 없이는 스케줄링 불가. PCB 저장/복원이 컨텍스트 스위칭(Context Switching)이고, 이 비용이 라운드 로빈(Round Robin, RR)의 타임 퀀텀(Time Quantum) 선택을 지배함(W06).

### 🔗 연결 6: MLFQ = SJF 의 실용적 근사

SJF 는 이론적 최적이지만 미래를 알아야 함(W06). MLFQ 는 **과거 행동을 관찰해서** SJF 를 근사함(W07). 이것이 현대 스케줄러의 근간.

### 🔗 연결 7: 공정성 vs 효율성 — 끝없는 트레이드오프

- FCFS(단순) vs SJF(효율) — 호위 효과(Convoy Effect) vs 기아(Starvation)
- 친화도(성능) vs 부하 분산(Load Balancing)(공정성)
- RMS(단순) vs EDF(100% 이용률)

**모든 스케줄링 결정은 이 두 목표의 타협**. 만능 해답은 없고 워크로드 특성에 맞춰야 함.

## 8.3 답안 작성 심화 팁

### 패턴 1 — "왜" 를 먼저

❌ "이중 모드(Dual-Mode)는 커널(Kernel)/사용자 모드(User Mode)를 갖는다."

✅ "단일 모드 CPU 라면 사용자 프로그램이 디스크를 직접 제어해 OS 를 파괴할 수 있다. 그래서 모드 비트를 하드웨어 상태 레지스터에 두고, 하드웨어가 전환을 강제함으로써 소프트웨어 트릭으로 우회할 수 없게 했다."

### 패턴 2 — 트레이드오프 언급

❌ "RR 은 타임 퀀텀(Time Quantum)이 중요하다."

✅ "RR 의 타임 퀀텀이 너무 작으면 컨텍스트 스위칭(Context Switching) 비용이 지배적이고, 너무 크면 FCFS 로 수렴해 응답 시간(Response Time)이 악화된다. 경험적으로 CPU 버스트(CPU Burst)의 80%가 한 퀀텀 내 완료되는 값이 좋다."

### 패턴 3 — 개념 연결

❌ "스레드(Thread) 간 스위칭은 빠르다."

✅ "스레드 간 컨텍스트 스위칭(Context Switching)이 프로세스(Process) 간보다 빠른 이유는 **TLB 플러시가 불필요** 하기 때문이다. 같은 주소 공간을 공유하므로 TLB 엔트리가 그대로 유효하다. 이것은 11~12주차 가상 메모리(Virtual Memory)의 주소 변환 계층과 직접 연결되는 설계다."

### 패턴 4 — 한계·예외

❌ "SJF 는 평균 대기 시간(Waiting Time)을 최소화한다."

✅ "SJF 는 이론적으로 평균 대기 시간을 최소화하지만, 현실에서는 다음 CPU 버스트(CPU Burst) 길이를 정확히 알 수 없으므로 지수 평균(Exponential Averaging) τ = αt + (1-α)τ 로 예측한다. 또한 긴 프로세스(Process)가 무한 대기(기아(Starvation))할 수 있어 노화(Aging)가 필수이다."

### 패턴 5 — 실무 예시

❌ "공유 메모리(Shared Memory)는 빠른 IPC 다."

✅ "공유 메모리의 대표적 실무 사례는 PostgreSQL 의 공유 버퍼 풀이다. 여러 연결이 같은 물리 메모리를 매핑해 8KB 페이지 복사를 피한다. 또한 Chromium 의 렌더러↔GPU 텍스처 공유로 4K 렌더링에서 제로 카피를 달성한다."

---

# 제9장. 시험 직전 체크리스트

## 9.1 반드시 답할 수 있어야 할 30가지 질문

### 정의·기본 (W01)
1. OS 란 무엇인가? (커널(Kernel) 중심)
2. 이중 모드(Dual-Mode)가 없다면 어떤 문제가 발생하는가?
3. 시스템 콜(System Call)과 라이브러리 함수의 차이는?
4. 트랩(Trap)과 하드웨어 인터럽트(Interrupt)의 차이는?
5. 모놀리식(Monolithic) vs 마이크로커널(Microkernel)의 트레이드오프는?

### 프로세스 (W02)
6. 프로세스(Process)와 프로그램의 차이는?
7. BSS 를 Data 와 분리하는 이유는?
8. PCB 의 주요 필드 4가지는?
9. fork() 반환값이 자식 0, 부모 PID 인 이유는?
10. fork + exec 분리 설계의 장점은?
11. 좀비(Zombie)와 고아(Orphan)의 차이와 각각의 해결 방법은?
12. 스레드(Thread) 간 컨텍스트 스위치(Context Switch)가 프로세스 간보다 빠른 이유는?

### IPC (W03)
13. 공유 메모리(Shared Memory)와 메시지 전달(Message Passing)의 트레이드오프는?
14. 유한 버퍼(Bounded Buffer)에서 왜 SIZE−1 개만 저장하는가?
15. 파이프(Pipe)에서 사용 안 하는 끝을 close 해야 하는 이유는?
16. RPC 의 exactly-once 의미론이 필요한 경우는?
17. 마샬링(Marshalling)이 필요한 이유(엔디안(Endianness) 문제)?

### 스레드 (W04)
18. 멀티스레딩의 4가지 이점은?
19. 동시성(Concurrency)과 병렬성(Parallelism)의 차이는?
20. 암달의 법칙(Amdahl's Law)이 시사하는 바는?
21. 일대일(One-to-One) 모델이 표준이 된 이유는?
22. 경쟁 조건(Race Condition)은 왜 발생하며 `partial_sum` 배열 해결책의 원리는?

### 암묵적 스레딩 (W05)
23. 스레드 풀(Thread Pool)의 3가지 이점은?
24. OpenMP reduction 이 내부적으로 어떻게 경쟁 조건(Race Condition)을 피하는가?
25. fork() 후 즉시 exec() 가 안전한 이유(고아 락(Orphaned Lock))는?
26. TLS 가 지역 변수와 다른 점은?

### 스케줄링 기본 (W06)
27. FCFS 의 호위 효과(Convoy Effect)를 예시로 설명하라.
28. SJF 가 최적인 이유와 현실적 장벽은?
29. RR 타임 퀀텀(Time Quantum)이 너무 크거나 작으면 어떤 일이 벌어지는가?

### 스케줄링 고급 (W07)
30. MLFQ 가 SJF 를 어떻게 근사하며 기아(Starvation)를 방지하는가?

## 9.2 계산 문제 연습

### SJF 계산 🧪
프로세스(Process): P1(6), P2(8), P3(7), P4(3), 모두 t=0 도착.

- 실행 순서: P4 → P1 → P3 → P2
- 대기: P4=0, P1=3, P3=9, P2=16
- 평균: 7

### RR 계산 🧪
프로세스(Process): P1(24), P2(3), P3(3), q=4, 모두 t=0 도착.

- 간트: P1(0-4) P2(4-7) P3(7-10) P1(10-14) P1(14-18) P1(18-22) P1(22-26) P1(26-30)
- P1 대기: 30-24=6, P2 대기: 4, P3 대기: 7
- 평균: 5.67

### 지수 평균(Exponential Averaging) 🧪
α=0.5, τ0=10. 실제 버스트 t0=6.

$$ \tau_1 = 0.5 \cdot 6 + 0.5 \cdot 10 = 8 $$

다음 실제 t1 = 4:

$$ \tau_2 = 0.5 \cdot 4 + 0.5 \cdot 8 = 6 $$

### 암달의 법칙(Amdahl's Law) 🧪
S = 20% (80% 병렬), N = 8 코어.

$$ \text{speedup} = \frac{1}{0.2 + 0.8/8} = \frac{1}{0.3} \approx 3.33 $$

N→∞: 1/0.2 = **5배 최대**.

### Little's 🧪
대화형 시스템에서 평균 큐 길이 n=20, 대기 시간(Waiting Time) W=0.5초 관찰.

$$ \lambda = n/W = 20/0.5 = 40 \text{ 프로세스(Process)/초} $$

## 9.3 코드 패턴 연습

### fork + exec + wait
```c
if ((pid = fork()) == 0) {
    execlp("ls", "ls", "-l", NULL);
    perror("exec"); exit(1);
} else if (pid > 0) {
    wait(NULL);
}
```

### Pipe ls | wc -l
```c
int fd[2]; pipe(fd);
if (fork() == 0) {
    close(fd[0]); dup2(fd[1], 1); close(fd[1]);
    execlp("ls", "ls", NULL);
}
if (fork() == 0) {
    close(fd[1]); dup2(fd[0], 0); close(fd[0]);
    execlp("wc", "wc", "-l", NULL);
}
close(fd[0]); close(fd[1]);
wait(NULL); wait(NULL);
```

### Pthread 안전 인자 전달
```c
int ids[N];
for (int i = 0; i < N; i++) {
    ids[i] = i;
    pthread_create(&t[i], NULL, f, &ids[i]);
}
```

### OpenMP reduction
```c
int sum = 0;
#pragma omp parallel for reduction(+:sum)
for (int i = 0; i < N; i++) sum += a[i];
```

## 9.4 시험 팁

### 답안 작성 6요소 템플릿 (보너스용)
```
[정의 1~2문장: 한글(영어)]
+ [왜 필요한가 — 대안/반례]
+ [메커니즘 2~4문장]
+ [구체적 예시 1개 (코드/계산/실무)]
+ [한계/트레이드오프 1문장]
+ [다른 개념과 연결 1문장 — 보너스!]
```

### 자주 하는 실수 방지
- [ ] fork() 반환값: 자식 0, 부모 PID (순서 주의)
- [ ] exec() 성공 후 코드: 실행 안 됨
- [ ] Pipe: 사용 안 하는 끝 **모두** close (부모도!)
- [ ] `write(fd, msg, strlen(msg)+1)` — null 포함
- [ ] pthread_create 인자: 루프의 `&i` 는 race, `&ids[i]` 사용
- [ ] Thread.start() vs run(): run() 직접은 같은 스레드(Thread)
- [ ] SJF 최적이지만 예측 필요·기아(Starvation) 위험
- [ ] RR 퀀텀: 80% 규칙 언급
- [ ] 암달 상한 1/S 언급

---

**Last Updated**: 2026-04-23
