# Midterm Summary Sheet

[Repository Guide](../../README.md) | [English](Summary-Sheet.md) | [한국어](Summary-Sheet.ko.md)

> **Last Updated:** 2026-10-20

---

## Table of Contents

- [1. Glossary](#1-glossary)
- [2. Week 1: Introduction to the OS](#2-week-1-introduction-to-the-os)
- [3. Week 2: Process Basics](#3-week-2-process-basics)
- [4. Week 3: IPC](#4-week-3-ipc)
- [5. Week 4: Threads and Concurrency](#5-week-4-threads-and-concurrency)
- [6. Week 5: Implicit Threading and Threading Issues](#6-week-5-implicit-threading-and-threading-issues)
- [7. Week 6: Basic CPU Scheduling Algorithms](#7-week-6-basic-cpu-scheduling-algorithms)
- [8. Week 7: Advanced Scheduling](#8-week-7-advanced-scheduling)
- [9. API Cheat Sheet](#9-api-cheat-sheet)
- [10. Formulas and Calculations](#10-formulas-and-calculations)
- [11. Frequently Tested Comparison Tables](#11-frequently-tested-comparison-tables)
- [12. Common Pitfalls](#12-common-pitfalls)
- [13. Final Strategy for the Exam](#13-final-strategy-for-the-exam)

---

<br>

## 1. Glossary

| English | Korean | One-Line Definition |
|---|---|---|
| Kernel | 커널(Kernel) | The always-running core of the OS, manager of the hardware |
| Dual-Mode | 이중 모드(Dual-Mode) | Protects the CPU with user and kernel modes |
| Privileged Instruction | 특권 명령어(Privileged Instruction) | Executable only in kernel mode |
| Trap | 트랩(Trap) | A synchronous interrupt raised by software |
| Interrupt | 인터럽트 | An asynchronous signal from a device to the CPU |
| System Call | 시스템 콜(System Call) | The only user→kernel interface |
| DMA | 직접 메모리 접근(Direct Memory Access, DMA) | Memory↔device transfer without the CPU |
| Context Switch | 컨텍스트 스위칭(Context Switching) | Saving/restoring process state |
| PCB | 프로세스 제어 블록(Process Control Block, PCB) | Kernel structure holding process information |
| Process | 프로세스 | A program in execution |
| Thread | 스레드(Thread) | A unit of execution within a process |
| fork() | - | Duplicates a process (creates a child) |
| exec() | - | Replaces the process image |
| wait() | - | Waits for a child to terminate |
| Zombie | 좀비(Zombie) | A terminated process that has not been wait()ed for |
| Orphan | 고아(Orphan) | A child whose parent terminated first |
| IPC | 프로세스 간 통신(Inter-Process Communication, IPC) | Data exchange between cooperating processes |
| Shared Memory | 공유 메모리(Shared Memory) | Fast IPC (needs synchronization) |
| Message Passing | 메시지 전달(Message Passing) | Safe IPC (send/receive) |
| Pipe | 파이프(Pipe) | One-way IPC channel |
| FIFO / Named Pipe | 명명 파이프(Named Pipe / FIFO) | File-system-based pipe |
| Socket | 소켓(Socket) | Network communication endpoint (IP+Port) |
| RPC | 원격 프로시저 호출(Remote Procedure Call, RPC) | Calls a remote function as if it were local |
| Marshalling | 마샬링(Marshalling) | Converts data into a transmission format |
| Concurrency | 동시성(Concurrency) | Multiple tasks in progress (interleaving) |
| Parallelism | 병렬성(Parallelism) | Truly simultaneous execution (multicore) |
| Amdahl's Law | 암달의 법칙(Amdahl's Law) | speedup ≤ 1/(S+(1-S)/N) |
| Race Condition | 경쟁 조건(Race Condition) | Bug caused by non-atomic shared access |
| Mutex | 뮤텍스(Mutex) | Mutual exclusion lock |
| TLB | 변환 참조 버퍼 | Cache for virtual→physical address translation |
| COW | Copy-on-Write | fork optimization (copy on write) |
| TLS | 스레드 로컬 저장소(Thread-Local Storage, TLS) | Per-thread independent global variables |
| LWP | 경량 프로세스(Lightweight Process, LWP) | Intermediate layer between user and kernel threads |
| PCS / SCS | 경쟁 범위 | Within a process / system-wide |
| SMP | 대칭 다중 프로세싱(Symmetric Multiprocessing, SMP) | All CPUs equal, independent scheduling |
| NUMA | 비균등 메모리 접근(Non-Uniform Memory Access, NUMA) | Memory latency differs per core |
| Affinity | 친화도 | Tendency to stay on the same CPU |
| FCFS | 선착순(First-Come First-Served, FCFS) | Arrival order (non-preemptive) |
| SJF | 최단 작업 우선(Shortest Job First, SJF) | Shortest burst first |
| SRTF | 최단 남은 시간(Shortest Remaining Time First, SRTF) | Preemptive SJF |
| RR | 라운드 로빈(Round Robin, RR) | Rotation by time quantum |
| MLQ | 다단계 큐(Multilevel Queue, MLQ) | Static classification, no movement between queues |
| MLFQ | 다단계 피드백 큐(Multilevel Feedback Queue, MLFQ) | Dynamic classification, demotion and promotion |
| Aging | 노화(Aging) | Priority rises with waiting time |
| Convoy Effect | 호위 효과(Convoy Effect) | In FCFS, a long job blocks the jobs behind it |
| Starvation | 기아(Starvation) | A process waits indefinitely |
| Dispatcher | 디스패처(Dispatcher) | Performs the actual context switch |
| RMS | Rate-Monotonic | Shorter period, higher priority (real-time) |
| EDF | Earliest-Deadline-First | Nearest deadline first |
| Priority Inversion | 우선순위 역전(Priority Inversion) | A low-priority task indirectly blocks a high-priority task |

*Table 1. English-Korean glossary for the midterm*

---

<br>

## 2. Week 1: Introduction to the OS

### OS = Kernel

- **Kernel:** Resident in memory from boot to shutdown, manager of hardware resources
- **System Program** (shell, compiler): User mode, distributed with the OS
- **Application Program:** Installed by the user

### Two Roles of the OS
| Role | Focus |
|---|---|
| Resource allocator | Fair and efficient distribution (CPU, memory, I/O) |
| Control program | Preventing errors and misuse (isolation, protection) |

*Table 2. The two roles of the OS*

### Dual-Mode
| Mode | Bit | Privilege |
|---|---|---|
| Kernel Mode | 0 | All instructions, all memory |
| User Mode | 1 | Restricted |

*Table 3. Mode bits and privileges of dual mode*

- Privileged instructions: **I/O, interrupt control, timer setting, page table changes, mode bit changes**
- Mode switches happen **only through traps/interrupts/returns** → enforced by hardware

### System Call Path (RISC-V)
```text
user code
  ↓ libc wrapper
  ↓ a7 ← syscall#, args ← a0-a5
  ↓ ecall (trap)
kernel trap handler
  ↓ syscall table lookup
sys_xxx() handler
  ↓ do the work, result → a0
  ↓ sret
user code
```

*Figure 1. System call path on RISC-V*

### Interrupt vs Trap
| | Interrupt | Trap |
|---|---|---|
| Source | External device (asynchronous) | Software (synchronous) |
| Examples | Disk, timer, keyboard | syscall, division by zero |

*Table 4. Comparison of interrupts and traps*

### DMA
Memory↔device transfer without CPU involvement. The CPU only hands the work command to DMA → DMA performs it → completion interrupt

### Storage Hierarchy
```text
Register < L1/L2 Cache < RAM < SSD < HDD
```

*Figure 2. Storage hierarchy*

Locality = **temporal** (reaccess) + **spatial** (neighbors)

### OS Structure
| | Advantages | Disadvantages | Examples |
|---|---|---|---|
| Monolithic | Fast | One bug→whole system down | Linux |
| Microkernel | Isolation, easier verification | Context switch overhead | Mach, QNX |
| Hybrid | Combines the advantages | Complex | macOS, Windows |

*Table 5. Advantages, disadvantages, and examples of OS structures*

---

<br>

## 3. Week 2: Process Basics

### Memory Layout
```text
[↑ high address]
Stack   ─ grows down (function calls)
  ↕
Heap    ─ grows up (malloc)
BSS     ─ uninitialized globals (zero-initialized automatically)
Data    ─ initialized globals
Text    ─ executable code
[↓ low address]
```

*Figure 3. Memory layout of a process*

### Process States
```text
New ─admitted→ Ready ─dispatch→ Running ─exit→ Terminated
                ↑      ↑              │
           I/O done interrupt      ↓
               └─Waiting ←─I/O req──┘
```

*Figure 4. Process state transition diagram*

| Transition | Cause |
|---|---|
| New→Ready | Memory allocation complete |
| Ready→Running | Selected by the scheduler |
| Running→Ready | **Timer interrupt (involuntary)** |
| Running→Waiting | **I/O request (voluntary)** |
| Waiting→Ready | I/O complete |
| Running→Terminated | exit() |

*Table 6. Process state transitions and their causes*

### Main PCB Fields
- **Process state**
- **PC (Program Counter)**
- **CPU registers**
- **CPU scheduling information** (priority)
- **Memory management information** (base/limit, page table)
- **Accounting information** (PID, CPU time)
- **I/O information** (list of open files)

### fork() Return Value Convention
| | Return |
|---|---|
| Child | **0** |
| Parent | **Child PID** (positive) |
| Error | -1 |

*Table 7. Return values of fork()*

### fork + exec + wait Pattern
```c
pid_t pid = fork();
if (pid == 0) {                          // child
    execlp("/bin/ls", "ls", "-l", NULL);
    perror("exec"); exit(1);             // not reached if exec succeeds
} else if (pid > 0) {                    // parent
    wait(NULL);                          // wait for the child to exit
}
```

### Zombie vs Orphan
| | Zombie | Orphan |
|---|---|---|
| Child state | **Terminated** (exit) | Running |
| Cause | Parent does not call wait() | Parent terminates first |
| Resolution | Parent calls wait() / kill the parent so init adopts it | Automatically adopted by init/systemd |
| Accumulation risk | **Yes** (process table exhaustion) | No |

*Table 8. Comparison of zombie and orphan processes*

### exec() Function Family (l=list, v=vector, p=PATH, e=env)
| Function | Arguments | Path | Environment |
|---|---|---|---|
| `execl` | list | full path | Inherited |
| `execlp` | list | PATH search | Inherited |
| `execle` | list | full path | Specified |
| `execv` | vector | full path | Inherited |
| `execvp` | vector | PATH search | Inherited |
| `execve` | vector | full path | Specified |

*Table 9. Argument, path, and environment passing in the exec() family*

### wait() Status Macros
| Macro | Purpose |
|---|---|
| `WIFEXITED(s)` | Exited normally? |
| `WEXITSTATUS(s)` | Exit code (0-255) |
| `WIFSIGNALED(s)` | Terminated by a signal? |
| `WTERMSIG(s)` | Terminating signal number |

*Table 10. wait() status macros*

### Context Switch
- A few microseconds
- **Pure overhead** (no useful work)
- Overhead factors: memory speed, number of registers, presence of special instructions
- Optimizations: multiple register sets, tagged TLB (ASID)

---

<br>

## 4. Week 3: IPC

### Shared Memory vs Message Passing
| | Shared Memory | Message Passing |
|---|---|---|
| Speed | Fast (no kernel after the initial setup) | Slow (a syscall on every call) |
| Synchronization | Programmer | OS |
| Suitable for | Large data, local | Small data, distributed |
| Implementation | Page tables share the same physical frames | Copying through kernel buffers |

*Table 11. Comparison of shared memory and message passing*

### Producer-Consumer Bounded Buffer
- Empty: `in == out`
- Full: `((in + 1) % SIZE) == out`
- Maximum stored: **SIZE − 1** items
- Why one slot stays empty: to distinguish empty from full

### Three Design Axes of Message Passing
| Axis | Options |
|---|---|
| Naming | Direct symmetric / direct asymmetric / indirect (mailbox) |
| Synchronization | Blocking send and recv / nonblocking (4 combinations) |
| Buffering | 0 (rendezvous) / bounded / unbounded |

*Table 12. Design axes and options of message passing*

### POSIX Shared Memory
```c
int fd = shm_open("name", O_CREAT|O_RDWR, 0666);
ftruncate(fd, SIZE);
char *ptr = mmap(0, SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
// work...
munmap(ptr, SIZE);
shm_unlink("name");
// compile: gcc -lrt
```

### Pipe
```c
int fd[2];
pipe(fd);           // fd[0]=read, fd[1]=write
// child:
close(fd[0]);       // always close the unused end!
write(fd[1], msg, strlen(msg)+1);  // +1 = include the null terminator
close(fd[1]);
// parent:
close(fd[1]);
read(fd[0], buf, BUFSIZ);
close(fd[0]);
```

**Mandatory rule:** Close **all** unused ends. The read side reaches EOF only after every write end is closed. Otherwise it **blocks forever**.

### Named Pipe (FIFO)
```c
mkfifo("/tmp/my_fifo", 0666);  // create
int fd = open("/tmp/my_fifo", O_WRONLY);
```

| | Ordinary Pipe | Named Pipe |
|---|---|---|
| Relationship | Parent-child | Unrelated processes possible |
| Lifetime | Until the process terminates | Persistent in the file system |
| Direction | One-way | UNIX: half-duplex / Windows: full-duplex |

*Table 13. Comparison of ordinary pipes and named pipes*

### dup2() Redirection
```c
int fd = open("out.txt", O_WRONLY|O_CREAT|O_TRUNC, 0644);
dup2(fd, STDOUT_FILENO);   // stdout → out.txt
close(fd);
printf("redirected");       // written to out.txt
```

### Key Points for Implementing Shell `ls | wc -l`
1. `pipe(fd)`
2. Child 1: `dup2(fd[1], STDOUT)`, close both ends, `execlp("ls"...)`
3. Child 2: `dup2(fd[0], STDIN)`, close both ends, `execlp("wc", "wc", "-l", NULL)`
4. Parent: **close both fds**, `wait × 2`

### Socket
- Combination of IP + Port, local loopback = `127.0.0.1`
- Well-known: 22(SSH) 80(HTTP) 443(HTTPS) 53(DNS) 25(SMTP) 21(FTP)
- TCP (connection-oriented, reliable) vs UDP (connectionless, fast)

### RPC
- **Stub** (client-side proxy) ↔ **Skeleton** (server side)
- **Marshalling:** Solves endianness and data representation problems (XDR)
- Semantics: **at-least-once** (safe only for idempotent operations) / **at-most-once** / **exactly-once** (required for finance)
- Modern: gRPC = Protocol Buffers + HTTP/2

---

<br>

## 5. Week 4: Threads and Concurrency

### Shared vs Per-Thread
| Shared | Per-Thread |
|---|---|
| Code, data, heap, open files | PC, registers, **stack**, thread ID |

*Table 14. Resources shared among threads and resources private to each thread*

### Why Threads Are Faster Than Processes
- Creation: only the stack and registers are initialized
- Context switch: **same address space → no TLB flush needed**
- Communication: global variables (no IPC needed)

### Four Benefits of Multithreading
1. **Responsiveness** (the UI responds during blocking)
2. **Resource Sharing** (direct)
3. **Economy** (cheap creation and switching)
4. **Scalability** (uses multiple cores)

### Concurrency vs Parallelism
| | Single Core | Multicore |
|---|---|---|
| Concurrency | O (interleaving) | O |
| Parallelism | **X** | O |

*Table 15. Concurrency and parallelism on single-core and multicore systems*

**Parallelism ⊂ Concurrency**

### Amdahl's Law
$$ \text{speedup} \le \frac{1}{S + \frac{1-S}{N}} $$

As N→∞, **speedup → 1/S** (upper bound)

| S | N=2 | N=4 | N=8 | N→∞ |
|---|---|---|---|---|
| 5% | 1.90 | 3.48 | 5.93 | **20** |
| 10% | 1.82 | 3.08 | 4.71 | **10** |
| 25% | 1.60 | 2.28 | 2.91 | **4** |
| 50% | 1.33 | 1.60 | 1.78 | **2** |

*Table 16. Speedup by serial fraction S and number of cores N*

### Types of Parallelism
| Data Parallelism | Task Parallelism |
|---|---|
| Same operation on distributed data | Different operations on multiple threads |
| Array summation | Web browser (download/render/input) |

*Table 17. Data parallelism and task parallelism*

### Multithreading Models
| Model | Mapping | Drawback | Current Use |
|---|---|---|---|
| Many-to-One | N:1 | Blocking propagates, no parallelism | Not used |
| **One-to-One** | 1:1 | Limit on the number of threads | **Linux, Windows** |
| Many-to-Many | N:M | Complex implementation (upcalls) | Go, Java virtual threads |
| Two-Level | N:M + 1:1 | Complex | Old Solaris |

*Table 18. Mapping, drawbacks, and use of multithreading models*

### Core Pthreads API
```c
int pthread_create(pthread_t*, pthread_attr_t*,
                   void*(*)(void*), void*);
int pthread_join(pthread_t, void**);
void pthread_exit(void*);
int pthread_attr_init(pthread_attr_t*);
```

### Race Condition and Its Solutions
```c
global_sum += x;  // LOAD-ADD-STORE is not atomic → lost update
```
**Solutions:**
- Separate storage `partial_sum[id]` (first practical choice)
- Mutex `pthread_mutex_lock/unlock`
- Atomic operations

### Argument-Passing Pitfall
```c
// ❌ Dangerous: all threads share the same &i
for (int i = 0; i < N; i++)
    pthread_create(&t[i], NULL, f, &i);

// ✅ Correct
int ids[N];
for (int i = 0; i < N; i++) {
    ids[i] = i;
    pthread_create(&t[i], NULL, f, &ids[i]);
}
```

### Java Threads
```java
// Recommended: implement Runnable (extending Thread blocks other inheritance)
new Thread(() -> { /* lambda */ }).start();

// Executor framework
ExecutorService pool = Executors.newFixedThreadPool(4);
Future<Integer> f = pool.submit(new Callable<Integer>() {...});
int ans = f.get();
pool.shutdown();
```

`start()` = new thread / calling `run()` directly = same thread (bug)

---

<br>

## 6. Week 5: Implicit Threading and Threading Issues

### Five Techniques **T-F-O-G-I**
| Technique | Language | Key Point |
|---|---|---|
| **T**hread pool | Java, Windows | execute/submit, Callable/Future |
| **F**ork-Join | Java | RecursiveTask, work stealing |
| **O**penMP | C/C++/Fortran | `#pragma omp parallel for reduction(+:s)` |
| **G**CD | macOS/iOS | dispatch queue + QoS |
| **I**ntel TBB | C++ | `parallel_for`, `parallel_reduce` |

*Table 19. Five implicit threading techniques*

### Three Advantages of Thread Pools
1. **Speed** (reuse)
2. **Resource limits** (upper bound)
3. **Separation** (task vs execution)

### Java Executor Factories
| Method | Purpose |
|---|---|
| `newSingleThreadExecutor()` | Size 1, sequential |
| `newFixedThreadPool(n)` | Fixed n |
| `newCachedThreadPool()` | Dynamic |

*Table 20. Java Executor factory methods*

### Main OpenMP Directives
```c
#pragma omp parallel                    // parallel region
#pragma omp parallel for                // split the loop
#pragma omp parallel for reduction(+:s) // safely combine the sum
// clauses: shared(), private(), reduction(op:var)
// operators: + - * & | ^ && || max min
// functions: omp_get_thread_num(), omp_get_num_threads(), omp_get_wtime()
// compile: gcc -fopenmp
```

> **Note:** The operators supported by the OpenMP `reduction` clause are `+`, `-`, `*`, `&`, `|`, `^`, `&&`, `||`, `max`, and `min`; division `/` is not supported.

### GCD QoS
| Class | Purpose |
|---|---|
| USER_INTERACTIVE | Immediate response (UI) |
| USER_INITIATED | User is waiting |
| UTILITY | Long-running BG |
| BACKGROUND | Indexing, backup |

*Table 21. GCD QoS classes*

### Five Threading Issues
#### ① fork() / exec()
- POSIX: **only the calling thread is duplicated**
- Another thread holds a mutex → risk that the child inherits an **orphaned lock**
- Safe: call exec immediately after fork

#### ② Signals
- **Synchronous signals** (SIGSEGV) → the thread that caused them
- **Asynchronous** (SIGTERM, SIGINT) → 4 options
- **Recommended:** block all of them, and a dedicated thread calls `sigwait()`

#### ③ Thread Cancellation
| | Asynchronous | **Deferred (default)** |
|---|---|---|
| Timing | Immediate | Only at cancellation points |
| Safety | Dangerous | Safe |

*Table 22. Comparison of asynchronous and deferred cancellation*

Cancellation points: blocking syscalls, `pthread_testcancel()`, `pthread_join`, etc.
Cleanup: `pthread_cleanup_push/pop(1)`

#### ④ TLS
```c
__thread int tls_var;              // GCC
_Thread_local int v;                // C11
ThreadLocal<Integer> t;             // Java
```
An independent copy per thread. `errno`, transaction ID, etc.

#### ⑤ Scheduler Activation and LWP
- LWP: a virtual CPU for the user thread library
- **Upcall:** kernel → notifies the user library of an event
- Many-to-many complexity → modern OSes choose one-to-one

### Linux clone()
| Flag | Shared |
|---|---|
| `CLONE_VM` | Virtual memory |
| `CLONE_FS` | File system information |
| `CLONE_FILES` | fd table |
| `CLONE_SIGHAND` | Signal handlers |
| `CLONE_THREAD` | Thread group |
| `CLONE_NEWNS/PID/NET` | Namespaces (containers) |

*Table 23. clone() flags and what they share*

- `fork()` = `clone(0)` (nothing shared)
- pthread_create = `clone(VM|FS|FILES|SIGHAND|THREAD)`

### Windows Thread Structures
| Structure | Location | Contents |
|---|---|---|
| ETHREAD | Kernel | Start address, parent, KTHREAD→ |
| KTHREAD | Kernel | State, priority, kernel stack, TEB→ |
| TEB | User | TID, user stack, TLS array |

*Table 24. Data structures of a Windows thread*

---

<br>

## 7. Week 6: Basic CPU Scheduling Algorithms

### CPU-I/O Burst
A process = alternating CPU bursts ↔ I/O bursts. I/O-bound (many short bursts) / CPU-bound (few long bursts).

### When Scheduling Decisions Occur
1. Running → Waiting (non-preemptive)
2. Running → Ready (preemptive)
3. Waiting → Ready (preemption possible)
4. Termination

**Preemptive:** possible beyond 1 and 4 | **Non-preemptive:** only 1 and 4

### Three Roles of the Dispatcher
1. Context switching
2. Switching between user and kernel mode
3. Jumping to the PC

**Dispatch Latency** = time from stop→start

### Scheduling Criteria
| Criterion | Meaning | Direction |
|---|---|---|
| CPU Utilization | Fraction of time the CPU is used | Maximize |
| Throughput | Completions per unit time | Maximize |
| Turnaround Time | Submission to completion | Minimize |
| **Waiting Time** | Time spent in the ready queue | **Minimize (primary)** |
| Response Time | Submission to first response | Minimize (interactive) |

*Table 25. Scheduling criteria and optimization directions*

Turnaround = waiting + burst + I/O. What scheduling affects is **waiting**.

### FCFS (First-Come First-Served, Non-preemptive)
- Simple, easy to implement
- **Convoy Effect:** if a long job is in front, all the short jobs behind it wait → average waiting↑
- Extremely sensitive to arrival order

**Example:** P1(24) P2(3) P3(3)
- Order 1,2,3: average waiting = (0+24+27)/3 = **17**
- Order 2,3,1: average waiting = (0+3+6)/3 = **3**

### SJF (Shortest Job First)
- **Theoretically optimal** (minimizes average waiting)
- Practical barrier: **the next burst must be predicted** → exponential averaging
- **Risk of starvation** → solved with aging

**Example:** P1(6), P2(8), P3(7), P4(3), all arrive at t=0
```text
| P4 | P1    | P3     | P2      |
0    3      9       16        24
```

*Figure 5. Gantt chart of the SJF example*

Waiting: P4=0, P1=3, P3=9, P2=16 → **average 7**

**Exponential averaging:**
$$ \tau_{n+1} = \alpha t_n + (1-\alpha)\tau_n $$
- α=0 → fixed on the past / α=1 → only the latest / usually α=0.5

### SRTF (Preemptive SJF)
- New arrival's burst < current remaining time → preemption
- Average waiting ↓ compared with SJF
- Starvation risk ↑

**Example:** P1(0,8) P2(1,4) P3(2,9) P4(3,5), where the parentheses are (arrival, burst)
```text
| P1 | P2     | P4     | P1      | P3       |
0    1       5       10       17        26
```

*Figure 6. Gantt chart of the SRTF example*

Waiting: P1=9, P2=0, P3=15, P4=2 → **average 6.5**

### Round Robin (RR)
- Each process gets a **time quantum** (usually 10-100ms)
- Quantum exceeded → preemption → to the end of the queue
- **No starvation:** maximum wait of a process = (N-1)×q
- **Good response time** (interactive)

**Example:** P1(24) P2(3) P3(3), q=4, all arrive at t=0
```text
| P1 | P2 | P3 | P1 | P1 | P1 | P1 | P1 |
0    4    7   10   14   18   22   26   30
```

*Figure 7. Gantt chart of the RR (q=4) example*

Waiting: P1=6, P2=4, P3=7 → **average 5.67**

**Choosing the time quantum:**
- Too large → converges to FCFS
- Too small → context switching overhead
- **80% rule:** 80% of bursts finish within one quantum

### Priority Scheduling
- Priority number (lower often means higher priority)
- Both preemptive and non-preemptive are possible
- SJF is the special case where priority = 1/next burst
- **Starvation → aging** solves it (the longer a process waits, the higher its priority ↑)

### Algorithm Summary
| | Type | Average Waiting | Response | Starvation |
|---|---|---|---|---|
| FCFS | Non-preemptive | Large (convoy effect) | Poor | None |
| SJF | Non-preemptive | **Minimum** | Good | Yes |
| SRTF | Preemptive | Even smaller than SJF | Good | Yes |
| RR | Preemptive | Large (time slice) | **Very good** | None |
| Priority | Both | Depends on policy | Depends on policy | Yes (→aging) |

*Table 26. Characteristics of basic scheduling algorithms*

---

<br>

## 8. Week 7: Advanced Scheduling

### Multilevel Queue (MLQ)
- Processes are **permanently assigned** by type (foreground-RR / background-FCFS)
- Moving between queues is **not allowed**
- Scheduling between queues: fixed priority or time slice distribution

### Multilevel Feedback Queue (MLFQ)
- **Three rules:**
  1. New process → top queue
  2. Quantum used up → **demotion** to a lower queue
  3. Long wait → **promotion** to a higher queue (aging)
- **Automatic classification** by observing behavior → approximates SJF
- Short jobs and I/O bound = upper / CPU bound = lower, separating naturally
- Default in modern UNIX-like systems

### Thread Scheduling Scope
| | Meaning | Models Using It |
|---|---|---|
| **PCS** | Competition within a process | Many-to-one, many-to-many |
| **SCS** | System-wide competition | One-to-one (**modern standard**) |

*Table 27. Thread scheduling scopes PCS and SCS*

### Multiprocessor Scheduling
#### SMP
Each CPU makes scheduling decisions independently. Adopted by most modern systems

#### Processor Affinity
- Stay on the same CPU → **cache preserved**
- Soft (when possible) / Hard (enforced)

#### NUMA
Memory latency differs per core → place the work on a core near its local memory

#### Load Balancing
- **Push migration:** overloaded → pushes tasks to another CPU
- **Pull migration:** idle → pulls tasks from a busy queue
- **Affinity vs load balancing conflict** → a balance is needed

#### SMT/Hyperthreading
Multiple HW threads on one physical core. Another thread runs during a memory stall.
**Scheduling hint:** Allocate **a different physical core first** before the second HW thread of the same physical core.

### Real-Time Scheduling
#### Soft vs Hard
- Soft: best effort (video, games)
- Hard: deadlines guaranteed (avionics, medical)

#### Rate-Monotonic (RMS)
- **Fixed priority = 1/period**
- Shorter period, higher priority
- **Utilization bound:** $U \le N(2^{1/N} - 1)$ → **69.3%** as N→∞

#### EDF (Earliest-Deadline-First)
- **Dynamic priority = nearest deadline**
- **100%** utilization is possible in theory
- Requires deadline information

#### Priority Inversion and the Inheritance Protocol
- T3 (low) holds a mutex + T1 (high) waits + T2 (medium) preempts T3 → T1 is indirectly blocked by T2
- **Solution:** temporarily raise the priority of the blocking task (priority inheritance)

### Linux CFS
- **vruntime** = virtual run time (weighted by priority)
- Sorted in an **RB-Tree** → O(log N) selection
- Meaning of Completely Fair: over the long run, CPU time is distributed in proportion to the weights

### Windows
- **32 priority levels**
- Dynamic boost: favors keyboard/focus windows → interactive responsiveness

### Four Ways to Evaluate Scheduling
| Method | Accuracy | Cost | Remarks |
|---|---|---|---|
| Deterministic modeling | Exact | Low | Only for a specific workload |
| Queueing model (Little's: n=λW) | General | Medium | Hard to compute for complex policies |
| Simulation | High | High | Synthetic/trace workloads |
| **Implementation** | **Perfect** | Very high | Evaluation can break down as user behavior changes |

*Table 28. Comparison of methods for evaluating scheduling algorithms*

**Little's Formula:** $n = \lambda \times W$
- n: average queue length
- λ: arrival rate
- W: average waiting time

Example: λ=7/sec, n=14 → W=2sec

---

<br>

## 9. API Cheat Sheet

### Process
```c
pid_t fork(void);                     // 0=child, >0=child PID, <0=error
int execlp(const char *file, ...);    // searches PATH, NULL-terminated
int execvp(const char *file, char *const argv[]);
pid_t wait(int *status);              // wait for a child to exit
pid_t waitpid(pid_t, int *, int);     // specific child
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

// Mutex (Week 9)
int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
int pthread_mutex_lock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);

// Condition Variable (Week 9)
int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
int pthread_cond_signal(pthread_cond_t *);
int pthread_cond_broadcast(pthread_cond_t *);

// Signal
int pthread_kill(pthread_t, int sig);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
```

### Compile Options
```bash
gcc -Wall -pthread -o prog prog.c          # pthreads
gcc -Wall -fopenmp -o prog prog.c          # OpenMP
gcc -Wall -lrt -o prog prog.c              # POSIX RT (shm_open)
# environment variable
OMP_NUM_THREADS=4 ./prog                   # number of OpenMP threads
```

---

<br>

## 10. Formulas and Calculations

### Amdahl's Law
$$ \text{speedup} \le \frac{1}{S + \frac{1-S}{N}} $$
- S: serial fraction, N: number of cores
- N→∞: speedup → 1/S

### SJF Exponential Averaging
$$ \tau_{n+1} = \alpha t_n + (1-\alpha)\tau_n $$

### Little's Formula
$$ n = \lambda W $$

### Rate-Monotonic Utilization Bound
$$ U \le N(2^{1/N} - 1) $$
≈ **0.693** (69.3%) as N→∞

### Procedure for Computing Average Waiting Time
1. Draw the Gantt chart
2. Read each process's **completion time**
3. Turnaround time = completion − arrival
4. Waiting time = turnaround − burst
5. Average = sum / number of processes

### Context Switch Overhead Factors
- Number of registers (more → ↑)
- Memory speed
- Special HW instructions
- TLB flush (only between processes)
- Cache effects

---

<br>

## 11. Frequently Tested Comparison Tables

### Process vs Thread
| | Process | Thread |
|---|---|---|
| Memory | Independent | Shared (except the stack) |
| Creation cost | High | Low |
| Context switch | Slow (TLB flush) | Fast |
| Communication | Requires IPC | Global variables |
| Isolation | Strong | Weak |

*Table 29. Comparison of processes and threads*

### fork vs clone vs pthread_create
| | Shared |
|---|---|
| `fork()` = `clone(0)` | Nothing |
| `pthread_create` ≈ `clone(VM\|FS\|FILES\|SIGHAND\|THREAD)` | Almost everything |
| Container = `clone(... \|NEWNS\|NEWPID\|NEWNET)` | Partial |

*Table 30. Sharing scope of fork, clone, and pthread_create*

### Shared Memory vs Message Passing vs Pipe vs Socket
| | Scope | Speed | Complexity |
|---|---|---|---|
| Shared memory | Local | Very fast | Synchronize it yourself |
| Message passing | Local/distributed | Slow | Managed by the OS |
| Pipe | Local (related) | Moderate | Easy |
| Named Pipe | Local (unrelated) | Moderate | Easy |
| Socket | Local/remote | Slow | Moderate |

*Table 31. Scope, speed, and complexity of IPC methods*

### Scheduling Algorithm Summary
| | Type | Average Waiting | Response | Starvation | Advantage | Disadvantage |
|---|---|---|---|---|---|---|
| FCFS | Non-preemptive | Large | Poor | X | Simple | Convoy effect |
| SJF | Non-preemptive | **Minimum** | Moderate | O | Optimal | Unpredictable |
| SRTF | Preemptive | Even smaller | Good | O | ↓ vs SJF | Overhead |
| RR | Preemptive | Large | **Very good** | X | Fair, responsive | Choosing the quantum |
| Priority | Both | Variable | Variable | O (→aging) | Reflects importance | Starvation |
| MLQ | Variable | Variable | Variable | O | Per-type policies | Fixed classification |
| MLFQ | Preemptive | Low | Good | X (promotion) | Automatic classification | Complex |
| RMS | Preemptive | - | - | - | Static, predictable | 69.3% utilization |
| EDF | Preemptive | - | - | - | 100% utilization | Requires deadlines |

*Table 32. Overall comparison of the strengths and weaknesses of scheduling algorithms*

### Choosing a Multithreading Model
| Situation | Model |
|---|---|
| Most modern OSes | One-to-one |
| Go language runtime | Many-to-many |
| Lots of blocking I/O | One-to-one or many-to-many |
| Very many threads | Many-to-many |

*Table 33. Multithreading model choice by situation*

### Choosing RPC Semantics
| Operation Type | Semantics |
|---|---|
| Query (idempotent) | at-least-once |
| Financial transaction (non-idempotent) | **exactly-once** |
| Event notification | at-most-once |

*Table 34. RPC semantics by operation type*

---

<br>

## 12. Common Pitfalls

1. **fork() return value:** The child gets 0, and the parent gets the child's PID. Do not mix up the order
2. **Code after a successful exec():** Not executed (memory is replaced)
3. **A zombie cannot be killed:** It is already dead, so the parent must be handled
4. **Pipe close rule:** Close **all** unused ends (including in the parent)
5. **`write(fd, msg, strlen(msg))` vs `sizeof(msg)`:** For a pointer, sizeof gives the pointer size
6. **`fd[WRITE_END]` and `+1`:** Send the null terminator so that `printf("%s")` works correctly
7. **`pthread_create(&t, NULL, f, &i)` in a loop:** The address of `i` is shared → race. Fix it with an `ids[i]` array
8. **Thread.start() vs run():** Calling run() directly runs it in the same thread (bug)
9. **Mutex inheritance after fork():** If another thread was holding it, the child can never release it
10. **FCFS convoy effect:** Even for the same set of processes, the average waiting time varies greatly with **arrival order**
11. **RR time quantum:** Too small means overhead, and too large turns it into FCFS. Apply the 80% rule
12. **SJF starvation:** A long process may wait forever → solved with aging
13. **RMS 69.3% bound:** Above this bound, RMS no longer guarantees schedulability, so consider EDF
14. **Process vs thread context switch:** The key to the speed difference = **whether the TLB is flushed**
15. **Amdahl's Law:** Even infinite cores are bounded by **1/S**. With S=50%, 2x is the limit
16. **One-to-one vs many-to-many:** The modern standard is **one-to-one**, because of simplicity and stability
17. **RPC exactly-once:** Financial transactions always need a unique transaction ID
18. **Named pipe:** Exists in the file system → unrelated processes can communicate
19. **Key to shared memory performance:** **No kernel involvement** after the initial setup
20. **Mode switch for system calls:** `ecall` (trap) → automatically kernel mode, `sret` → automatically user mode

---

<br>

## 13. Final Strategy for the Exam

### Template for a Deep-Understanding Answer
```text
[1 sentence of definition] + [1 sentence on why it was designed this way]
+ [2-3 sentences on mechanism/how it works]
+ [1 concrete example]
+ [1 sentence on a tradeoff or limitation]
+ [1 sentence connecting to another concept]
```

### Example: Answer to a Question on Why System Calls Are Needed

A system call is the only interface through which a user program requests OS services. On a single-mode CPU, a program could manipulate I/O or memory mappings directly and destroy the OS, so dual mode was introduced and entry into kernel mode was restricted to happen **only through a trap**.

In the call sequence, the libc wrapper puts the system call number in register a7 (RISC-V) and executes the `ecall` instruction, and the hardware then changes the mode bit to 0 and jumps to the trap handler. For example, an `open()` call internally raises ecall, runs the kernel's `sys_open()`, and returns with the fd in a0.

The limitation of this structure is **overhead**. Even a simple command like `cp` issues thousands of system calls, so buffering, DMA, and mmap become important. This connects directly to memory-mapped files (`mmap()`), which build on virtual memory.
