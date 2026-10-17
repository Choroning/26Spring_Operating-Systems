# 운영체제 중간고사 Summary Sheet

---

## 목차 빠른 조회
1. [용어 사전 (영↔한)](#-1-용어-사전)
2. [W01 OS 소개](#-w01-os-소개)
3. [W02 프로세스(Process) 기초](#-w02-프로세스-기초)
4. [W03 IPC](#-w03-ipc)
5. [W04 스레드(Thread)·동시성(Concurrency)](#-w04-스레드동시성)
6. [W05 암묵적 스레딩(Implicit Threading)·스레딩 이슈](#-w05-암묵적-스레딩-스레딩-이슈)
7. [W06 CPU 스케줄링 기본](#-w06-cpu-스케줄링-기본-알고리즘)
8. [W07 고급 스케줄링](#-w07-고급-스케줄링)
9. [자주 쓰는 API 치트시트](#-api-치트시트)
10. [공식·계산 모음](#-공식계산-모음)
11. [시험 단골 비교표](#-시험-단골-비교표)
12. [답안 작성 체크리스트](#-답안-작성-체크리스트)

---

## 📒 1. 용어 사전

| 영문 | 한국어 | 1줄 정의 |
|---|---|---|
| Kernel | 커널(Kernel) | 항상 실행 중인 OS 핵심, 하드웨어 관리자 |
| Dual-Mode | 이중 모드(Dual-Mode) | 사용자/커널 모드(Kernel Mode)로 CPU 보호 |
| Privileged Instruction | 특권 명령어(Privileged Instruction) | 커널 모드에서만 실행 가능 |
| Trap | 트랩(Trap) | 소프트웨어 발생 동기 인터럽트(Interrupt) |
| Interrupt | 인터럽트 | 장치가 CPU에 보내는 비동기 신호 |
| System Call | 시스템 콜(System Call) | 유저→커널 유일 인터페이스 |
| DMA | 직접 메모리 접근(Direct Memory Access, DMA) | CPU 없이 메모리↔장치 전송 |
| Context Switch | 컨텍스트 스위칭(Context Switching) | 프로세스(Process) 상태 저장/복원 |
| PCB | 프로세스 제어 블록(Process Control Block, PCB) | 프로세스 정보 담는 커널 구조체 |
| Process | 프로세스 | 실행 중인 프로그램 |
| Thread | 스레드(Thread) | 프로세스 내 실행 단위 |
| fork() | — | 프로세스 복제 (자식 생성) |
| exec() | — | 프로세스 이미지 교체 |
| wait() | — | 자식 종료 대기 |
| Zombie | 좀비(Zombie) | 종료됐으나 wait()되지 않은 프로세스 |
| Orphan | 고아(Orphan) | 부모가 먼저 종료된 자식 |
| IPC | 프로세스 간 통신(Inter-Process Communication, IPC) | 협력 프로세스의 데이터 교환 |
| Shared Memory | 공유 메모리(Shared Memory) | 빠른 IPC (동기화 필요) |
| Message Passing | 메시지 전달(Message Passing) | 안전한 IPC (send/receive) |
| Pipe | 파이프(Pipe) | 단방향 IPC 채널 |
| FIFO / Named Pipe | 명명 파이프(Named Pipe / FIFO) | 파일 시스템 기반 파이프 |
| Socket | 소켓(Socket) | 네트워크 통신 끝점 (IP+Port) |
| RPC | 원격 프로시저 호출(Remote Procedure Call, RPC) | 원격 함수를 로컬처럼 호출 |
| Marshalling | 마샬링(Marshalling) | 데이터를 전송 형식으로 변환 |
| Concurrency | 동시성(Concurrency) | 여러 작업 진행 중 (인터리빙) |
| Parallelism | 병렬성(Parallelism) | 실제 동시 실행 (멀티코어) |
| Amdahl's Law | 암달의 법칙(Amdahl's Law) | speedup ≤ 1/(S+(1-S)/N) |
| Race Condition | 경쟁 조건(Race Condition) | 비원자적 공유 접근 버그 |
| Mutex | 뮤텍스(Mutex) | 상호 배제 잠금 |
| TLB | 변환 참조 버퍼 | 가상→물리 주소 변환 캐시 |
| COW | Copy-on-Write | fork 최적화 (쓰기 시 복사) |
| TLS | 스레드-로컬 저장소(Thread-Local Storage, TLS) | 스레드별 독립 전역 변수 |
| LWP | 경량 프로세스(Lightweight Process, LWP) | 유저/커널 스레드(Kernel Thread) 중간 계층 |
| PCS / SCS | 경쟁 범위 | 프로세스 내 / 시스템 전체 |
| SMP | 대칭 다중 프로세싱(Symmetric Multiprocessing, SMP) | 모든 CPU 동등·독립 스케줄링 |
| NUMA | 비균등 메모리 접근(Non-Uniform Memory Access, NUMA) | 코어마다 메모리 지연 다름 |
| Affinity | 친화도 | 같은 CPU 유지 경향 |
| FCFS | 선착순(First-Come First-Served, FCFS) | 도착 순 (비선점(Non-preemptive)) |
| SJF | 최단 작업 우선(Shortest Job First, SJF) | 짧은 버스트 우선 |
| SRTF | 최단 남은 시간(Shortest Remaining Time First, SRTF) | SJF 선점형(Preemptive) |
| RR | 라운드 로빈(Round Robin, RR) | 타임 퀀텀(Time Quantum) 순환 |
| MLQ | 다단계 큐(Multilevel Queue, MLQ) | 정적 분류, 큐 이동 불가 |
| MLFQ | 다단계 피드백 큐(Multilevel Feedback Queue, MLFQ) | 동적 분류, 강등(Demotion)·승격(Promotion) |
| Aging | 노화(Aging) | 대기 시간(Waiting Time)에 따라 우선순위 상승 |
| Convoy Effect | 호위 효과(Convoy Effect) | FCFS에서 긴 작업이 앞을 막음 |
| Starvation | 기아(Starvation) | 프로세스 무한 대기 |
| Dispatcher | 디스패처(Dispatcher) | 실제 컨텍스트 스위치(Context Switch) 수행 |
| RMS | Rate-Monotonic | 주기(Period) 짧을수록 우선 (실시간) |
| EDF | Earliest-Deadline-First | 가장 가까운 마감(Deadline) 우선 |
| Priority Inversion | 우선순위 역전(Priority Inversion) | 낮은 작업이 높은 작업 간접 블록 |

---

## 🖥️ W01 OS 소개

### OS = 커널(Kernel)

- **커널(Kernel)**: 부팅~종료 메모리 상주, 하드웨어 자원 관리자
- **시스템 프로그램(System Program)** (셸·컴파일러): 유저 모드, OS와 함께 배포
- **응용 프로그램(Application Program)**: 사용자 설치

### OS의 2가지 역할
| 역할 | 초점 |
|---|---|
| 자원 할당자 | 공정·효율 분배 (CPU·메모리·I/O) |
| 제어 프로그램 | 오류·오용 방지 (격리·보호) |

### 이중 모드(Dual-Mode)
| 모드 | 비트 | 권한 |
|---|---|---|
| 커널 모드(Kernel Mode) | 0 | 모든 명령·메모리 |
| 사용자 모드(User Mode) | 1 | 제한 |

- 특권 명령어(Privileged Instruction): **I/O, 인터럽트(Interrupt) 제어, 타이머 설정, 페이지 테이블(Page Table) 변경, 모드 비트 변경**
- 모드 전환은 **트랩(Trap)/인터럽트/리턴** 로만 가능 → 하드웨어 강제

### 시스템 콜(System Call) 경로 (RISC-V)
```
user code
  ↓ libc wrapper
  ↓ a7 ← syscall#, 인자 ← a0~a6
  ↓ ecall (트랩)
kernel trap handler
  ↓ syscall table lookup
sys_xxx() handler
  ↓ 작업 수행, 결과 → a0
  ↓ sret
user code
```

### 인터럽트(Interrupt) vs 트랩(Trap)
| | 인터럽트(Interrupt) | 트랩(Trap) |
|---|---|---|
| 발생 | 외부 장치 (비동기) | 소프트웨어 (동기) |
| 예 | 디스크·타이머·키보드 | syscall, 0으로 나누기 |

### DMA
CPU 개입 없이 메모리↔장치 전송. CPU에 "작업 명령" → DMA가 수행 → 완료 인터럽트(Interrupt)

### 저장 계층
```
Register < L1/L2 Cache < RAM < SSD < HDD
```
지역성(Locality) = **시간적** (재접근) + **공간적** (인접)

### OS 구조
| | 장점 | 단점 | 예 |
|---|---|---|---|
| Monolithic | 빠름 | 한 버그→전체 다운 | Linux |
| Microkernel | 격리·검증 | 컨텍스트 스위치(Context Switch) 오버헤드 | Mach, QNX |
| Hybrid | 장점 결합 | 복잡 | macOS, Windows |

---

## ⚙️ W02 프로세스(Process) 기초

### 메모리 레이아웃
```
[↑ 높은 주소]
Stack   ─ 아래로 성장 (함수 호출)
  ↕
Heap    ─ 위로 성장 (malloc)
BSS     ─ 초기화 안 된 전역 (0으로 자동 초기화)
Data    ─ 초기화된 전역
Text    ─ 실행 코드
[↓ 낮은 주소]
```

### 프로세스(Process) 상태
```
New ─admitted→ Ready ─dispatch→ Running ─exit→ Terminated
                ↑      ↑              │
           I/O완료 interrupt       ↓
               └─Waiting ←─I/O요청──┘
```

| 전이 | 원인 |
|---|---|
| New→Ready | 메모리 할당 완료 |
| Ready→Running | 스케줄러 선택 |
| Running→Ready | **타이머 인터럽트(Interrupt) (비자발)** |
| Running→Waiting | **I/O 요청 (자발)** |
| Waiting→Ready | I/O 완료 |
| Running→Terminated | exit() |

### PCB 주요 필드
- **프로세스(Process) 상태**
- **PC (프로그램 카운터(Program Counter, PC))**
- **CPU 레지스터**
- **CPU 스케줄링 정보** (우선순위)
- **메모리 관리 정보** (base/limit, page table)
- **계정 정보** (PID, CPU 시간)
- **I/O 정보** (열린 파일 목록)

### fork() 반환값 규약
| | 반환 |
|---|---|
| 자식 | **0** |
| 부모 | **자식 PID** (양수) |
| 오류 | -1 |

### fork + exec + wait 패턴
```c
pid_t pid = fork();
if (pid == 0) {                          // 자식
    execlp("/bin/ls", "ls", "-l", NULL);
    perror("exec"); exit(1);             // exec 성공하면 도달 X
} else if (pid > 0) {                    // 부모
    wait(NULL);                          // 자식 종료 대기
}
```

### 좀비(Zombie) vs 고아(Orphan)
| | 좀비(Zombie) | 고아(Orphan) |
|---|---|---|
| 자식 상태 | **종료됨** (exit) | 실행 중 |
| 원인 | 부모 wait() 미호출 | 부모가 먼저 종료 |
| 해결 | 부모가 wait() / 부모 죽여 init 입양 | init/systemd 자동 입양 |
| 누적 위험 | **있음** (프로세스(Process) 테이블 고갈) | 없음 |

### exec() 함수 계열 (l=list, v=vector, p=PATH, e=env)
| 함수 | 인자 | 경로 | 환경 |
|---|---|---|---|
| `execl` | list | full path | 상속 |
| `execlp` | list | PATH 검색 | 상속 |
| `execle` | list | full path | 지정 |
| `execv` | vector | full path | 상속 |
| `execvp` | vector | PATH 검색 | 상속 |
| `execve` | vector | full path | 지정 |

### wait() 상태 매크로
| 매크로 | 용도 |
|---|---|
| `WIFEXITED(s)` | 정상 종료? |
| `WEXITSTATUS(s)` | 종료 코드 (0~255) |
| `WIFSIGNALED(s)` | 시그널(Signal) 종료? |
| `WTERMSIG(s)` | 종료 시그널 번호 |

### 컨텍스트 스위치(Context Switch)
- 수 마이크로초
- **순수 오버헤드** (유용한 작업 X)
- 오버헤드 요인: 메모리 속도, 레지스터 수, 특수 명령어 유무
- 최적화: 다중 레지스터 세트, 태그드 TLB(ASID)

---

## 🔗 W03 IPC

### 공유 메모리(Shared Memory) vs 메시지 전달(Message Passing)
| | 공유 메모리(Shared Memory) | 메시지 전달(Message Passing) |
|---|---|---|
| 속도 | 빠름 (초기 설정 후 커널(Kernel) X) | 느림 (매 호출 syscall) |
| 동기화 | 프로그래머 | OS |
| 적합 | 대량 데이터, 로컬 | 소량, 분산 |
| 구현 | 페이지 테이블(Page Table)로 같은 물리 프레임 공유 | 커널 버퍼 복사 |

### 생산자(Producer)-소비자(Consumer) 유한 버퍼(Bounded Buffer)
- 빈 상태: `in == out`
- 가득: `((in + 1) % SIZE) == out`
- 최대 저장: **SIZE − 1** 개
- 왜 한 슬롯 비우나: 빈/가득 구분 위해

### 메시지 전달(Message Passing) 설계 3축
| 축 | 옵션 |
|---|---|
| 네이밍 | 직접 대칭 / 직접 비대칭 / 간접(메일박스(Mailbox)) |
| 동기화 | 블로킹 send·recv / 넌블로킹 (조합 4가지) |
| 버퍼링 | 0(랑데부(Rendezvous)) / 유한 / 무한 |

### POSIX 공유 메모리(Shared Memory)
```c
int fd = shm_open("name", O_CREAT|O_RDWR, 0666);
ftruncate(fd, SIZE);
char *ptr = mmap(0, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
// 작업...
munmap(ptr, SIZE);
shm_unlink("name");
// 컴파일: gcc -lrt
```

### 파이프(Pipe)
```c
int fd[2];
pipe(fd);           // fd[0]=read, fd[1]=write
// 자식:
close(fd[0]);       // 사용 안 할 끝 반드시 close!
write(fd[1], msg, strlen(msg)+1);  // +1 = null 포함
close(fd[1]);
// 부모:
close(fd[1]);
read(fd[0], buf, BUFSIZ);
close(fd[0]);
```

**필수 규칙**: 사용 안 할 끝은 **모두** close. 모든 쓰기 끝이 닫혀야 read 쪽에 EOF 도달. 안 그러면 **영구 블록**.

### 명명 파이프 (FIFO)
```c
mkfifo("/tmp/my_fifo", 0666);  // 생성
int fd = open("/tmp/my_fifo", O_WRONLY);
```
| | Ordinary Pipe | Named Pipe |
|---|---|---|
| 관계 | 부모-자식 | 무관한 프로세스(Process) 가능 |
| 수명 | 프로세스 종료 시 | 파일 시스템에 영구 |
| 방향 | 단방향 | UNIX: 반이중 / Windows: 전이중 |

### dup2() 리디렉션
```c
int fd = open("out.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644);
dup2(fd, STDOUT_FILENO);   // stdout → out.txt
close(fd);
printf("redirected");       // out.txt에 기록
```

### 셸 `ls | wc -l` 구현 요점
1. `pipe(fd)`
2. 자식1: `dup2(fd[1], STDOUT)`, close 양쪽, `execlp("ls"...)`
3. 자식2: `dup2(fd[0], STDIN)`, close 양쪽, `execlp("wc", "wc", "-l", NULL)`
4. 부모: **양쪽 fd 모두 close**, `wait × 2`

### 소켓(Socket)
- IP + Port 조합, 로컬 루프백 = `127.0.0.1`
- Well-known: 22(SSH) 80(HTTP) 443(HTTPS) 53(DNS) 25(SMTP) 21(FTP)
- TCP (연결·신뢰) vs UDP (비연결·빠름)

### RPC
- **스텁(Stub)** (클라이언트 측 프록시) ↔ **스켈레톤(Skeleton)** (서버 측)
- **마샬링(Marshalling)**: 엔디안(Endianness)·표현 문제 해결 (XDR)
- 의미론: **at-least-once** (멱등(Idempotent)만 안전) / **at-most-once** / **exactly-once** (금융 필수)
- 현대: gRPC = Protocol Buffers + HTTP/2

---

## 🧵 W04 스레드(Thread)·동시성(Concurrency)

### 스레드(Thread) 공유 vs 독립
| 공유 | 독립 |
|---|---|
| 코드·데이터·힙·열린 파일 | PC, 레지스터, **스택**, 스레드(Thread) ID |

### 스레드(Thread)가 프로세스(Process)보다 빠른 이유
- 생성: 스택·레지스터만 초기화
- 컨텍스트 스위치(Context Switch): **같은 주소 공간 → TLB 플러시 불필요**
- 통신: 전역 변수 (IPC 불필요)

### 멀티스레딩 4가지 이점
1. **응답성(Responsiveness)** (블로킹 중 UI 반응)
2. **자원 공유(Resource Sharing)** (직접)
3. **경제성(Economy)** (생성·전환 저비용)
4. **확장성(Scalability)** (멀티코어 활용)

### 동시성(Concurrency) vs 병렬성(Parallelism)
| | 단일 코어 | 멀티 코어 |
|---|---|---|
| 동시성(Concurrency) | O (인터리빙) | O |
| 병렬성(Parallelism) | **X** | O |

**병렬성 ⊂ 동시성**

### 암달의 법칙(Amdahl's Law)
$$ \text{speedup} \le \frac{1}{S + \frac{1-S}{N}} $$

N→∞ 에서 **speedup → 1/S** (상한)

**표**
| S | N=2 | N=4 | N=8 | N→∞ |
|---|---|---|---|---|
| 5% | 1.90 | 3.48 | 5.93 | **20** |
| 10% | 1.82 | 3.08 | 4.71 | **10** |
| 25% | 1.60 | 2.28 | 2.91 | **4** |
| 50% | 1.33 | 1.60 | 1.78 | **2** |

### 병렬성(Parallelism) 유형
| 데이터 병렬성(Data Parallelism) | 작업 병렬성(Task Parallelism) |
|---|---|
| 같은 연산 분산 데이터에 | 다른 연산 여러 스레드(Thread)에 |
| 배열 합산 | 웹 브라우저 (다운/렌더/입력) |

### 멀티스레딩 모델
| 모델 | 매핑 | 단점 | 현재 |
|---|---|---|---|
| 다대일(Many-to-One) | N:1 | 블로킹 전파, 병렬성(Parallelism) X | 미사용 |
| **일대일(One-to-One)** | 1:1 | 스레드(Thread) 수 제한 | **Linux, Windows** |
| 다대다(Many-to-Many) | N:M | 구현 복잡 (업콜(Upcall)) | Go, Java 가상 스레드 |
| 두 수준(Two-Level) | N:M + 1:1 | 복잡 | 구 Solaris |

### Pthreads 핵심 API
```c
int pthread_create(pthread_t*, pthread_attr_t*,
                   void*(*)(void*), void*);
int pthread_join(pthread_t, void**);
void pthread_exit(void*);
int pthread_attr_init(pthread_attr_t*);
```

### 경쟁 조건(Race Condition) & 해결
```c
global_sum += x;  // LOAD-ADD-STORE 비원자 → 업데이트 손실
```
**해결**:
- 별도 저장소 `partial_sum[id]` (실용적 1순위)
- 뮤텍스(Mutex) `pthread_mutex_lock/unlock`
- 원자적 연산(Atomic Operation)

### 인자 전달 함정
```c
// ❌ 위험: 모든 스레드가 같은 &i 공유
for (int i = 0; i < N; i++)
    pthread_create(&t[i], NULL, f, &i);

// ✅ 올바름
int ids[N];
for (int i = 0; i < N; i++) {
    ids[i] = i;
    pthread_create(&t[i], NULL, f, &ids[i]);
}
```

### Java 스레드(Thread)
```java
// 권장: Runnable 구현 (Thread 상속은 다중 상속 제한)
new Thread(() -> { /* 람다 */ }).start();

// Executor 프레임워크
ExecutorService pool = Executors.newFixedThreadPool(4);
Future<Integer> f = pool.submit(new Callable<Integer>() {...});
int ans = f.get();
pool.shutdown();
```

`start()` = 새 스레드(Thread) / `run()` 직접 = 같은 스레드 (버그)

---

## 🧩 W05 암묵적 스레딩(Implicit Threading)·스레딩 이슈

### 5가지 기법 **T-F-O-G-I**
| 기법 | 언어 | 핵심 |
|---|---|---|
| **T** hread pool | Java, Windows | execute/submit, Callable/Future |
| **F** ork-Join | Java | RecursiveTask, work stealing |
| **O** penMP | C/C++/Fortran | `#pragma omp parallel for reduction(+:s)` |
| **G** CD | macOS/iOS | dispatch queue + QoS |
| **I** ntel TBB | C++ | `parallel_for`, `parallel_reduce` |

### 스레드 풀(Thread Pool) 3가지 장점
1. **속도** (재사용)
2. **자원 제한** (상한)
3. **분리** (태스크 vs 실행)

### Java Executor 팩토리
| 메서드 | 용도 |
|---|---|
| `newSingleThreadExecutor()` | 크기 1, 순차 |
| `newFixedThreadPool(n)` | 고정 n |
| `newCachedThreadPool()` | 동적 |

### OpenMP 주요 지시어
```c
#pragma omp parallel                    // 병렬 영역
#pragma omp parallel for                // 루프 분할
#pragma omp parallel for reduction(+:s) // 합계 안전 결합
// 절: shared(), private(), reduction(op:var)
// 연산자: + - * / & | ^ && || max min
// 함수: omp_get_thread_num(), omp_get_num_threads(), omp_get_wtime()
// 컴파일: gcc -fopenmp
```

### GCD QoS
| 클래스 | 용도 |
|---|---|
| USER_INTERACTIVE | 즉시 응답 (UI) |
| USER_INITIATED | 사용자 대기 중 |
| UTILITY | 장시간 BG |
| BACKGROUND | 인덱싱·백업 |

### 스레딩 이슈 5가지
#### ① fork() / exec()
- POSIX: **호출 스레드(Thread)만 복제**
- 다른 스레드가 mutex 보유 중 → 자식이 **고아 락(Orphaned Lock)** 상속 위험
- 안전: fork 직후 즉시 exec

#### ② 시그널(Signal)
- **동기 시그널(Synchronous Signal)** (SIGSEGV) → 유발 스레드(Thread)
- **비동기** (SIGTERM, SIGINT) → 옵션 4가지
- **권장**: 모두 차단, 전담 스레드가 `sigwait()`

#### ③ 스레드(Thread) 취소
| | 비동기 | **지연(기본)** |
|---|---|---|
| 시점 | 즉시 | 취소 지점(Cancellation Point)에서만 |
| 안전 | 위험 | 안전 |

취소 지점: 블로킹 syscall, `pthread_testcancel()`, `pthread_join` 등
정리: `pthread_cleanup_push/pop(1)`

#### ④ TLS
```c
__thread int tls_var;              // GCC
_Thread_local int v;                // C11
ThreadLocal<Integer> t;             // Java
```
각 스레드(Thread) 독립 복사본. `errno`, transaction ID 등.

#### ⑤ 스케줄러 활성화(Scheduler Activation) & LWP
- LWP: 유저 스레드(Thread) 라이브러리의 가상 CPU
- **업콜(Upcall)**: 커널(Kernel) → 유저 라이브러리에 이벤트 알림
- 다대다(Many-to-Many) 복잡성 → 현대 OS는 일대일(One-to-One) 선택

### Linux clone()
| 플래그 | 공유 |
|---|---|
| `CLONE_VM` | 가상 메모리(Virtual Memory) |
| `CLONE_FS` | 파일시스템 정보 |
| `CLONE_FILES` | fd 테이블 |
| `CLONE_SIGHAND` | 시그널(Signal) 핸들러 |
| `CLONE_THREAD` | 스레드(Thread) 그룹 |
| `CLONE_NEWNS/PID/NET` | 네임스페이스(Namespace) (컨테이너(Container)) |

- `fork()` = `clone(0)` (공유 X)
- pthread_create = `clone(VM|FS|FILES|SIGHAND|THREAD)`

### Windows 스레드(Thread) 구조
| 구조 | 위치 | 내용 |
|---|---|---|
| ETHREAD | 커널(Kernel) | 시작주소, 부모, KTHREAD→ |
| KTHREAD | 커널 | 상태, 우선순위, 커널 스택, TEB→ |
| TEB | 유저 | TID, 유저 스택, TLS 배열 |

---

## 🗓️ W06 CPU 스케줄링 기본 알고리즘

### CPU-I/O 버스트(I/O Burst)
프로세스(Process) = CPU 버스트(CPU Burst) ↔ I/O 버스트(I/O Burst) 반복. I/O-bound (짧은 버스트 많음) / CPU-bound (긴 버스트 적음).

### 스케줄링 결정 시점
1. Running → Waiting (비선점(Non-preemptive))
2. Running → Ready (선점)
3. Waiting → Ready (선점 가능)
4. 종료

**선점형(Preemptive)**: 1·4 외에도 가능 | **비선점형(Non-preemptive)**: 1·4만

### 디스패처(Dispatcher) 3역할
1. 컨텍스트 스위칭(Context Switching)
2. 유저/커널 모드(Kernel Mode) 전환
3. PC 점프
**디스패치 지연(Dispatch Latency)** = 중단→시작 시간

### 스케줄링 기준
| 기준 | 의미 | 방향 |
|---|---|---|
| CPU 이용률(CPU Utilization) | CPU 사용 비율 | 최대 |
| 처리량(Throughput) | 단위 시간당 완료 수 | 최대 |
| 반환 시간 (Turnaround) | 제출~완료 | 최소 |
| **대기 시간(Waiting Time)** | 준비 큐(Ready Queue) 체류 | **최소 (주요)** |
| 응답 시간(Response Time) | 제출~첫 응답 | 최소 (대화형) |

반환 = 대기 + 버스트 + I/O. 스케줄링이 영향 주는 건 **대기**.

### FCFS (선착순(First-Come First-Served, FCFS), 비선점(Non-preemptive))
- 간단, 구현 쉬움
- **호위 효과(Convoy Effect)**: 긴 작업이 앞이면 뒤 짧은 작업들 전부 대기 → 평균 대기↑
- 도착 순서에 극도로 민감

**예시**: P1(24) P2(3) P3(3)
- 순서 1,2,3: 평균 대기 = (0+24+27)/3 = **17**
- 순서 2,3,1: 평균 대기 = (0+3+6)/3 = **3**

### SJF (최단 작업 우선(Shortest Job First, SJF))
- **이론적 최적** (평균 대기 최소화)
- 현실 장벽: **다음 버스트 예측 필요** → 지수 평균(Exponential Averaging)
- **기아(Starvation) 위험** → 노화(Aging)로 해결

**예시**: P1(6), P2(8), P3(7), P4(3), 모두 t=0 도착
```
| P4 | P1    | P3     | P2      |
0    3      9       16        24
```
대기: P4=0, P1=3, P3=9, P2=16 → **평균 7**

**지수 평균**:
$$ \tau_{n+1} = \alpha t_n + (1-\alpha)\tau_n $$
- α=0 → 과거 고정 / α=1 → 최근만 / 보통 α=0.5

### SRTF (SJF 선점형(Preemptive))
- 새 도착의 버스트 < 현재 남은 시간 → 선점(Preemption)
- SJF보다 평균 대기 ↓
- 기아(Starvation) 위험 ↑

**예시**: P1(0,8) P2(1,4) P3(2,9) P4(3,5) — (도착, 버스트)
```
| P1 | P2     | P4     | P1      | P3       |
0    1       5       10       17        26
```
대기: P1=9, P2=0, P3=15, P4=2 → **평균 6.5**

### Round Robin (RR)
- 각 프로세스(Process)에 **타임 퀀텀(Time Quantum)** 할당 (보통 10~100ms)
- 퀀텀 초과 → 선점(Preemption) → 큐 끝으로
- **기아(Starvation) 없음**: 한 프로세스 최대 대기 = (N-1)×q
- **응답 시간(Response Time) 우수** (대화형)

**예시**: P1(24) P2(3) P3(3), q=4, 모두 t=0 도착
```
| P1 | P2 | P3 | P1 | P1 | P1 | P1 | P1 |
0    4    7   10   14   18   22   26   30
```
대기: P1=6, P2=4, P3=7 → **평균 5.67**

**타임 퀀텀 선택**:
- 너무 크면 → FCFS로 수렴
- 너무 작으면 → 컨텍스트 스위칭(Context Switching) 오버헤드
- **80% 규칙**: 버스트의 80%가 한 퀀텀 내 완료

### 우선순위 스케줄링(Priority Scheduling)
- 우선순위 숫자 (낮을수록 높은 경우 많음)
- 선점(Preemption)/비선점 모두 가능
- SJF는 우선순위 = 1/다음 버스트의 특수 경우
- **기아(Starvation) → 노화(Aging)**로 해결 (대기할수록 우선순위 ↑)

### 알고리즘 요약
| | 유형 | 평균 대기 | 응답 | 기아(Starvation) |
|---|---|---|---|---|
| FCFS | 비선점(Non-preemptive) | 큼 (호위 효과(Convoy Effect)) | 나쁨 | 없음 |
| SJF | 비선점 | **최소** | 좋음 | 있음 |
| SRTF | 선점 | SJF보다 더 작음 | 좋음 | 있음 |
| RR | 선점 | 큼 (타임 슬라이스(Time Slice)) | **매우 좋음** | 없음 |
| Priority | 양쪽 | 정책 따라 | 정책 따라 | 있음 (→노화(Aging)) |

---

## 🎯 W07 고급 스케줄링

### 다단계 큐 (MLQ)
- 프로세스(Process)를 유형별 **영구 배정** (포그라운드-RR / 백그라운드-FCFS)
- 큐 이동 **불가**
- 큐 간 스케줄링: 고정 우선순위 또는 타임 슬라이스(Time Slice) 분배

### 다단계 피드백 큐 (MLFQ)
- **3가지 규칙**:
  1. 새 프로세스(Process) → 최상위 큐
  2. 퀀텀 소진 → 하위로 **강등(Demotion)**
  3. 오래 대기 → 상위로 **승격(Promotion)** (aging)
- 행동 관찰로 **자동 분류** → SJF 근사
- 짧은·I/O bound = 상위 / CPU bound = 하위 자연 분화
- 현대 UNIX 계열 기본

### 스레드(Thread) 스케줄링 범위
| | 의미 | 사용 모델 |
|---|---|---|
| **PCS** | 프로세스(Process) 내 경쟁 | 다대일(Many-to-One)·다대다(Many-to-Many) |
| **SCS** | 시스템 전체 경쟁 | 일대일(One-to-One) (**현대 표준**) |

### 멀티프로세서 스케줄링
#### SMP
각 CPU 독립 스케줄링 결정 — 대부분 현대 시스템

#### 프로세서 친화도(Processor Affinity)
- 같은 CPU 유지 → **캐시 보존**
- Soft (가능한) / Hard (강제)

#### NUMA
코어마다 메모리 지연 다름 → 로컬 메모리 근처 코어에 배치

#### 부하 분산(Load Balancing)
- **Push migration**: 과부하 → 다른 CPU로 밀어냄
- **Pull migration**: 유휴 → 바쁜 큐에서 끌어옴
- **친화도 vs 부하 분산(Load Balancing) 상충** → 균형 필요

#### SMT/하이퍼스레딩(Hyperthreading)
물리 코어 1개에 HW 스레드(Thread) 여러 개. 메모리 스톨 시 다른 스레드 실행.
**스케줄링 힌트**: 같은 물리 코어의 두 번째 HW 스레드보다 **다른 물리 코어 먼저** 할당.

### 실시간 스케줄링(Real-Time Scheduling)
#### Soft vs Hard
- Soft: 최선 노력 (비디오·게임)
- Hard: 마감(Deadline) 보증 (항공·의료)

#### Rate-Monotonic (RMS)
- **고정 우선순위 = 1/주기(Period)**
- 주기 짧을수록 우선
- **이용률 한계**: $U \le N(2^{1/N} - 1)$ → N→∞ 에서 **69.3%**

#### EDF (Earliest-Deadline-First)
- **동적 우선순위 = 가장 가까운 마감(Deadline)**
- 이론상 **100%** 이용률 가능
- 마감 정보 필요

#### Priority Inversion & 상속 프로토콜
- T3(낮) 뮤텍스(Mutex) 보유 + T1(높) 대기 + T2(중)이 T3 선점(Preemption) → T1이 T2에 간접 블록
- **해결**: 블로킹하는 태스크의 우선순위를 일시 상승 (우선순위 상속(Priority Inheritance))

### Linux CFS
- **vruntime** = 가상 실행 시간(Virtual Run Time, vruntime) (우선순위 가중)
- **RB-Tree**로 정렬 → O(log N) 선택
- "완전히 공정": 장기적으로 가중치 비율대로 CPU 분배

### Windows
- **32 우선순위 레벨**
- 동적 부스트: 키보드/포커스 윈도 우대 → 인터랙티브 응답성(Responsiveness)

### 스케줄링 평가 4방법
| 방법 | 정확도 | 비용 | 비고 |
|---|---|---|---|
| 결정론적 모델링(Deterministic Modeling) | 명확 | 낮음 | 특정 워크로드만 |
| 큐잉 모델 (Little's: n=λW) | 일반 | 중간 | 복잡 정책 계산 어려움 |
| 시뮬레이션(Simulation) | 높음 | 높음 | 합성/트레이스 워크로드 |
| **구현** | **완벽** | 매우 높음 | 사용자 행동 변화로 평가 무너짐 가능 |

**Little's Formula**: $n = \lambda \times W$
- n: 평균 큐 길이
- λ: 도착률
- W: 평균 대기

예: λ=7/sec, n=14 → W=2sec

---

## 🧰 API 치트시트

### 프로세스(Process)
```c
pid_t fork(void);                     // 0=자식, >0=자식PID, <0=오류
int execlp(const char *file, ...);    // PATH 검색, NULL 종결
int execvp(const char *file, char *const argv[]);
pid_t wait(int *status);              // 자식 종료 대기
pid_t waitpid(pid_t, int *, int);     // 특정 자식
void exit(int status);
pid_t getpid(void);
pid_t getppid(void);
```

### IPC
```c
// Pipe
int pipe(int fd[2]);                  // fd[0]=read, fd[1]=write
int mkfifo(const char *path, mode_t); // Named pipe

// Shared Memory
int shm_open(const char *name, int oflag, mode_t);
int ftruncate(int fd, off_t length);
void *mmap(void *addr, size_t length, int prot, int flags,
           int fd, off_t offset);
int munmap(void *, size_t);
int shm_unlink(const char *);

// I/O
int open(const char *path, int flags, mode_t);
ssize_t read(int fd, void *buf, size_t);
ssize_t write(int fd, const void *buf, size_t);
int close(int fd);
int dup2(int oldfd, int newfd);
```

### Pthreads
```c
int pthread_create(pthread_t *, const pthread_attr_t *,
                   void *(*)(void *), void *);
int pthread_join(pthread_t, void **);
void pthread_exit(void *);
pthread_t pthread_self(void);
int pthread_cancel(pthread_t);
void pthread_testcancel(void);
void pthread_cleanup_push(void (*)(void*), void *);
void pthread_cleanup_pop(int execute);

// Mutex (9주차)
int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
int pthread_mutex_lock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);

// Condition Variable (9주차)
int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
int pthread_cond_signal(pthread_cond_t *);
int pthread_cond_broadcast(pthread_cond_t *);

// Signal
int pthread_kill(pthread_t, int sig);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
```

### 컴파일 옵션
```bash
gcc -Wall -pthread -o prog prog.c          # pthreads
gcc -Wall -fopenmp -o prog prog.c          # OpenMP
gcc -Wall -lrt -o prog prog.c              # POSIX RT (shm_open)
# 환경변수
OMP_NUM_THREADS=4 ./prog                   # OpenMP 스레드(Thread) 수
```

---

## 📐 공식·계산 모음

### 암달의 법칙(Amdahl's Law)
$$ \text{speedup} \le \frac{1}{S + \frac{1-S}{N}} $$
- S: 순차 비율, N: 코어 수
- N→∞: speedup → 1/S

### SJF 지수 평균(Exponential Averaging)
$$ \tau_{n+1} = \alpha t_n + (1-\alpha)\tau_n $$

### Little's Formula
$$ n = \lambda W $$

### Rate-Monotonic 이용률 한계
$$ U \le N(2^{1/N} - 1) $$
N→∞ 에서 ≈ **0.693** (69.3%)

### 스케줄링 평균 대기 시간(Waiting Time) 계산 절차
1. 간트 차트(Gantt Chart) 그리기
2. 각 프로세스(Process) **완료 시각** 읽기
3. 반환 시간(Turnaround Time) = 완료 − 도착
4. 대기 시간(Waiting Time) = 반환 − 버스트
5. 평균 = 합 / 프로세스 수

### 컨텍스트 스위치(Context Switch) 오버헤드 요인
- 레지스터 수 (많을수록 ↑)
- 메모리 속도
- 특수 HW 명령
- TLB 플러시 (프로세스(Process) 간만)
- 캐시 효과

---

## 📊 시험 단골 비교표

### 프로세스(Process) vs 스레드(Thread)
| | 프로세스(Process) | 스레드(Thread) |
|---|---|---|
| 메모리 | 독립 | 공유 (스택 제외) |
| 생성 비용 | 높음 | 낮음 |
| 컨텍스트 스위치(Context Switch) | 느림 (TLB 플러시) | 빠름 |
| 통신 | IPC 필요 | 전역 변수 |
| 격리 | 강함 | 약함 |

### fork vs clone vs pthread_create
| | 공유 |
|---|---|
| `fork()` = `clone(0)` | 아무것도 |
| `pthread_create` ≈ `clone(VM\|FS\|FILES\|SIGHAND\|THREAD)` | 거의 전부 |
| Container = `clone(... \|NEWNS\|NEWPID\|NEWNET)` | 부분 |

### 공유 메모리(Shared Memory) vs 메시지 전달(Message Passing) vs 파이프(Pipe) vs 소켓(Socket)
| | 범위 | 속도 | 복잡도 |
|---|---|---|---|
| 공유 메모리(Shared Memory) | 로컬 | 매우 빠름 | 동기화 직접 |
| 메시지 전달(Message Passing) | 로컬/분산 | 느림 | OS 관리 |
| Pipe | 로컬 (관련) | 보통 | 쉬움 |
| Named Pipe | 로컬 (무관) | 보통 | 쉬움 |
| Socket | 로컬/원격 | 느림 | 보통 |

### 스케줄링 알고리즘 요약
| | 유형 | 평균 대기 | 응답 | 기아(Starvation) | 장점 | 단점 |
|---|---|---|---|---|---|---|
| FCFS | 비선점(Non-preemptive) | 큼 | 나쁨 | X | 단순 | 호위 효과(Convoy Effect) |
| SJF | 비선점 | **최소** | 보통 | O | 최적 | 예측 불가 |
| SRTF | 선점 | 더 작음 | 좋음 | O | SJF보다↓ | 오버헤드 |
| RR | 선점 | 큼 | **매우 좋음** | X | 공정, 응답성(Responsiveness) | 퀀텀 선택 |
| Priority | 양쪽 | 가변 | 가변 | O (→노화(Aging)) | 중요도 반영 | 기아 |
| MLQ | 가변 | 가변 | 가변 | O | 유형별 정책 | 고정 분류 |
| MLFQ | 선점 | 낮음 | 좋음 | X (승격(Promotion)) | 자동 분류 | 복잡 |
| RMS | 선점 | — | — | — | 정적·예측 | 이용률 69.3% |
| EDF | 선점 | — | — | — | 이용률 100% | 마감(Deadline) 필요 |

### 멀티스레딩 모델 선택
| 경우 | 모델 |
|---|---|
| 대부분의 현대 OS | 일대일(One-to-One) |
| Go 언어 런타임 | 다대다(Many-to-Many) |
| 블로킹 I/O 많음 | 일대일 또는 다대다 |
| 스레드(Thread) 수 매우 많음 | 다대다 |

### RPC 의미론 선택
| 연산 종류 | 의미론 |
|---|---|
| 조회 (멱등(Idempotent)) | at-least-once |
| 금융 거래 (비멱등) | **exactly-once** |
| 이벤트 알림 | at-most-once |

---

## 🔑 자주 나오는 "함정" 정리

1. **fork() 반환값**: 자식 0, 부모 PID — 순서 혼동 주의
2. **exec() 성공 후 코드**: 실행되지 않음 (메모리 교체됨)
3. **좀비(Zombie) kill 불가**: 이미 죽었음, 부모를 처리해야 함
4. **Pipe close 규칙**: 사용 안 하는 끝 **모두** close (부모도!)
5. **write(fd, msg, strlen(msg))** vs `sizeof(msg)`: 포인터면 포인터 크기 반환됨
6. **`fd[WRITE_END]` 와 `+1`**: null 포함해 전송해야 printf("%s") 정상 동작
7. **`pthread_create(&t, NULL, f, &i)` 루프**: `i` 주소 공유 → race. `ids[i]` 배열로 해결
8. **Thread.start() vs run()**: run() 직접 호출은 같은 스레드(Thread)에서 실행 (버그)
9. **fork() 후 mutex 상속**: 다른 스레드가 잡고 있었으면 자식은 영원히 풀 수 없음
10. **FCFS 호위 효과(Convoy Effect)**: 같은 프로세스(Process) 집합이라도 **도착 순서**에 따라 평균 대기가 크게 다름
11. **RR 타임 퀀텀(Time Quantum)**: 너무 작으면 오버헤드, 너무 크면 FCFS — 80% 규칙
12. **SJF 기아(Starvation)**: 긴 프로세스가 무한 대기 가능 → 노화(Aging)로 해결
13. **RMS 69.3% 한계**: 이 이상은 EDF로
14. **프로세스 vs 스레드 컨텍스트 스위치(Context Switch)**: 속도 차이의 핵심 = **TLB 플러시 여부**
15. **암달의 법칙(Amdahl's Law)**: 무한 코어도 **1/S**가 상한. S=50% 이면 2배가 한계
16. **일대일(One-to-One) vs 다대다(Many-to-Many)**: 현대 표준은 **일대일**, 이유는 단순성·안정성
17. **RPC exactly-once**: 금융 거래에는 반드시 고유 트랜잭션 ID 필요
18. **Named pipe**: 파일 시스템에 존재 → 무관한 프로세스 통신 가능
19. **공유 메모리(Shared Memory) 성능 핵심**: 초기 설정 후 **커널(Kernel) 미개입**
20. **시스템 콜(System Call) 모드 전환**: `ecall`(트랩(Trap)) → 자동으로 커널 모드(Kernel Mode), `sret` → 자동으로 유저 모드

---

## 📝 최종 시험 실전 전략

### "깊은 이해" 답안 템플릿
```
[정의 1문장] + [왜 이렇게 설계했나 1문장]
+ [메커니즘/작동 원리 2~3문장]
+ [구체적 예시 1개]
+ [트레이드오프 or 한계 1문장]
+ [다른 개념과 연결 1문장 — 보너스!]
```

### 예: "시스템 콜(System Call)이 왜 필요한가?" 답안
> 시스템 콜(System Call)은 사용자 프로그램이 OS 서비스를 요청하는 유일한 인터페이스다. 단일 모드 CPU라면 프로그램이 직접 I/O나 메모리 매핑을 조작해 OS를 파괴할 수 있기 때문에, 이중 모드(Dual-Mode)를 두고 커널 모드(Kernel Mode) 진입을 **트랩(Trap)** 으로만 가능하게 제한했다.
>
> 호출 과정은 libc 래퍼가 시스템 콜 번호를 a7 레지스터(RISC-V)에 넣고 `ecall` 명령을 실행하면 하드웨어가 모드 비트를 0으로 바꾸고 트랩 핸들러로 점프하는 방식이다. 예컨대 `open()` 호출은 내부적으로 ecall을 일으켜 커널(Kernel)의 `sys_open()` 을 실행시키고 fd를 a0에 담아 리턴한다.
>
> 이 구조의 한계는 **오버헤드** 다. `cp` 같은 단순 명령도 수천 번의 시스템 콜을 발생시키므로, 버퍼링·DMA·mmap이 중요해진다. 이는 12주차 가상 메모리(Virtual Memory)의 메모리 맵 파일과 직접 연결된다.

---

**Last Updated**: 2026-04-23
