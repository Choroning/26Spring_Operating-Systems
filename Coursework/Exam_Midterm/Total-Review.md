# Midterm Comprehensive Review

[Repository Guide](../../README.md) | [English](Total-Review.md) | [한국어](Total-Review.ko.md)

> **Last Updated:** 2026-10-20

---

## Table of Contents

- [Chapter 0. How to Use This Guide for Exam Preparation](#chapter-0-how-to-use-this-guide-for-exam-preparation)
- [Chapter 1. What Is an Operating System (Week 1)](#chapter-1-what-is-an-operating-system-week-1)
- [Chapter 2. The Birth and Life of a Process (Week 2)](#chapter-2-the-birth-and-life-of-a-process-week-2)
- [Chapter 3. Inter-Process Communication (IPC) (Week 3)](#chapter-3-inter-process-communication-ipc-week-3)
- [Chapter 4. Fundamentals of Threads and Concurrency (Week 4)](#chapter-4-fundamentals-of-threads-and-concurrency-week-4)
- [Chapter 5. Implicit Threading and Threading Issues (Week 5)](#chapter-5-implicit-threading-and-threading-issues-week-5)
- [Chapter 6. Basic CPU Scheduling Algorithms (Week 6)](#chapter-6-basic-cpu-scheduling-algorithms-week-6)
- [Chapter 7. Advanced Scheduling: Multiprocessor and Real-Time (Week 7)](#chapter-7-advanced-scheduling-multiprocessor-and-real-time-week-7)
- [Chapter 8. The Big Picture: How the Concepts of Seven Weeks Fit Together](#chapter-8-the-big-picture-how-the-concepts-of-seven-weeks-fit-together)
- [Chapter 9. Last-Minute Exam Checklist](#chapter-9-last-minute-exam-checklist)

---

## Chapter 0. How to Use This Guide for Exam Preparation

### 0.1 Study Order

1. **First pass**: Read Chapters 1 through 8 and check the **One-Line Summary** at the end of each section
2. **Second pass**: Work through the examples and calculations in each chapter **by hand**
3. **Third pass**: Read Chapter 8 (the links between concepts) several times to internalize **why these concepts exist together**
4. **The day before the exam**: Use the Chapter 9 checklist to find anything you missed

### 0.2 A Six-Element Template for Writing Deep-Understanding Answers

A **deep-understanding answer** includes the following six elements.

| Element | Description | Example |
|---|---|---|
| ① Definition | Korean term + English term + one sentence | "A process is a program in execution." |
| ② Why it is needed | Alternatives, counterexamples, motivation | "Without the process abstraction, only one program could run at a time." |
| ③ Mechanism | How it works | "Store the PC, registers, and memory information in the PCB, and switch with a context switch..." |
| ④ Concrete example | Code, calculation, practice | "Opening a browser creates a new process, each with its own independent memory..." |
| ⑤ Limitation/trade-off | Cost, constraints | "However, communication between processes requires IPC and has large overhead" |
| ⑥ Link to other concepts | Relation to other units | "This limitation is the motivation for introducing threads, and in Week 4..." |

*Table 1. Six elements of a deep-understanding answer*

### 0.3 Symbols Used in This Document

- 🎯 **Key concept**: a part you must understand
- 💡 **Intuition**: an analogy or a simple explanation
- ⚠️ **Pitfall**: a point where mistakes are common
- 🔗 **Connection**: the relation to other units
- 🧪 **Example/Calculation**: a part to work through by hand

---

## Chapter 1. What Is an Operating System (Week 1)

### 1.1 The Problem to Solve: Why an OS Is Needed

Computer hardware is a collection of devices such as the **CPU, memory, disk, network card, and keyboard**. Users want to use a word processor, listen to music, and open a browser. **But connecting these two worlds directly causes serious problems.**

- 🎯 **Problem 1**: Only one program can run at a time. To use the word processor, you must close the browser.
- 🎯 **Problem 2**: Each program controls the hardware directly → a single faulty program can erase the disk.
- 🎯 **Problem 3**: If a program uses memory however it likes, other programs' data gets corrupted.
- 🎯 **Problem 4**: Every programmer must know the detailed commands of each piece of hardware, so there is no portability at all.

The **operating system (OS)** is the **intermediate layer** that solves all of these problems.

```text
┌───────────────────────────┐
│  Application programs (Word, Browser...) │
├───────────────────────────┤
│  Operating system (OS)        │  ← the subject of this course
├───────────────────────────┤
│  Hardware (CPU, memory, disk...)│
└───────────────────────────┘
```

*Figure 1. Layered structure of application programs, the operating system, and hardware*

### 1.2 The Kernel: The Heart of the OS 🎯

**The core of the OS is the kernel.** If the exam asks "What is an OS?", this word must appear in the answer.

**Three defining properties of the kernel**:

1. **A program that is always running**: the only program that starts the moment the computer boots and never terminates until the power is turned off
2. **Hardware resource manager**: manages all hardware resources (CPU, memory, I/O) and provides services to application programs
3. **Privileged execution**: the only program that can execute privileged instructions, which are not accessible in user mode

💡 **Analogy (the OS is like a government)**: Just as a government does not make goods or provide services directly, the OS does no useful work by itself. A government provides public safety, infrastructure, and currency so that the market can run, and the OS provides resource management, protection, and scheduling so that application programs can run.

**Classification of OS components**:

| Category | Execution mode | Examples |
|---|---|---|
| **Kernel** | Kernel mode only | Process management, memory management, file system |
| **System program** | User mode | Shell (bash), compiler (gcc), ls, GUI |
| **Application** | User mode | Word processor, browser, games |

*Table 2. Classification of OS components*

System programs are distinguished from the kernel in that they **are distributed with the OS** but run in user mode. For example, the shell (bash) is included in Linux by default, but everything the shell does (command parsing, I/O redirection) happens in user mode. Only when the shell calls system calls such as `fork()`, `exec()`, and `dup2()` does the kernel do any work.

### 1.3 The Two Roles of the OS: Resource Allocator vs Control Program

The role of the OS can be viewed from two complementary perspectives.

#### 1.3.1 Resource Allocator

**How should limited resources be divided fairly and efficiently?**

What it manages:
- **CPU time**: which process runs when? → CPU scheduling (Weeks 6-7)
- **Memory space**: which region is allocated to each process? → memory management (Weeks 11-12)
- **Storage & I/O**: disk access order, device sharing → I/O scheduling

Key trade-off:
- **Fairness** vs **Efficiency**: must give every process a fair chance while maximizing overall throughput

#### 1.3.2 Control Program

**How can user programs be prevented from misusing the system?**

Functions it handles:
- **Execution management**: controls starting, stopping, and suspending programs
- **Error prevention**: catches illegal operations such as division by zero and unauthorized memory access
- **Isolation guarantee**: protects so that a bug in process A cannot touch process B or the kernel

🔗 **Connection**: These two roles are not separate. To divide resources fairly (resource allocator), the OS must be able to force a process not to monopolize a resource (control program), and for that the hardware must support two modes (dual mode in Section 1.4).

### 1.4 Dual-Mode Operation: How the OS Protects Itself 🎯

**The most important question**: If some user program tries to execute a command to "erase the entire disk", how can it be stopped?

Answer: **The hardware stops it physically.** This is the core of **dual-mode operation**.

#### 1.4.1 Two Execution Modes

| Mode | Mode bit | Who runs | Privileges |
|---|---|---|---|
| **Kernel mode (kernel/supervisor mode)** | 0 | OS kernel | All instructions, all memory access |
| **User mode** | 1 | Application programs | Restricted instructions, restricted memory |

*Table 3. Comparison of kernel mode and user mode*

**Physical location of the mode bit**: It is stored in the CPU's **status register**. In RISC-V, the current mode is not directly visible in a register; when a trap occurs, the mode before the trap is recorded in the SPP (Supervisor Previous Privilege) bit of the `sstatus` register and used by `sret` when returning.

#### 1.4.2 Privileged Instructions

A **privileged instruction** is a CPU instruction that can harm the system if misused, and it **can be executed only in kernel mode**. If it is attempted in user mode, **the hardware blocks it physically** and raises a **trap**.

Typical privileged instructions:
1. **I/O instructions**: direct access to the disk and network card
2. **Interrupt control**: enabling/disabling interrupts
3. **Timer setting**: controlling the CPU timer
4. **Memory management**: changing the page table
5. **Changing the mode bit**: switching user ↔ kernel

#### 1.4.3 Why the Hardware Must Enforce It

⚠️ **Pitfall**: This rule cannot be enforced by software. If the OS only warned "Do not execute bad instructions" and the hardware did not enforce it, a malicious program could simply ignore the warning. Because **the hardware blocks it physically**, no software trick can ever bypass it.

#### 1.4.4 Trap: An Intentional Interrupt

A **trap** is an **interrupt that software raises intentionally**. A program uses it to request help from the kernel.

How a trap occurs:
1. A user-mode program attempts to execute a privileged instruction or calls a system call
2. The CPU **raises a trap**
3. The hardware **automatically** sets the mode bit to 0 (kernel mode)
4. Control moves to the kernel's **trap handler**
5. The kernel handles the request and returns
6. The hardware restores the mode bit to 1 (user mode)

🎯 **Important properties**:
- **Enforced by hardware**: the hardware performs the mode switch, and software cannot change the mode arbitrarily
- **Every kernel entry is traceable**: entry happens only through an attempted privileged instruction or a system call, which helps security monitoring

💡 **Analogy (bank counter)**: Bank staff (kernel mode) can access the vault, while customers (user mode) can request services only through the counter (system call). If a customer tries to jump over the counter, a security guard stops them immediately = a trap occurs.

**One-Line Summary**: To protect itself, the OS relies on a CPU with two modes, and when a privileged instruction is attempted in user mode, the hardware forcibly blocks it with a trap.

### 1.5 System Call: The Only Door Between the User and the Kernel 🎯

If dual mode is the wall, a **system call** is the **only passage** that legitimately goes through the wall.

#### 1.5.1 What a System Call Is

**Definition**: The only mechanism by which a user program requests an OS service.

**Why it is needed**:
- A user program cannot execute privileged instructions directly
- But opening files, allocating memory, and printing to the screen ultimately require privileged instructions
- → A **standard protocol for having the kernel do work on its behalf** is needed = system call

💡 **Analogy (restaurant)**: A customer cannot enter the kitchen (kernel), so a waiter (system call) takes the order and passes it to the kitchen. The dish (service) is made with equipment the customer cannot touch (hardware) and then brought to the customer.

#### 1.5.2 Full Flow of a System Call (RISC-V)

The **detailed step-by-step path** of the call `open("file.txt", O_RDONLY)`:

```text
[1] User program
       ↓ looks similar to printf() but is different
[2] open() wrapper function in the C library
       ↓ sets the system call number in register a7
       ↓ sets the arguments in registers a0, a1, ...
[3] executes the ecall instruction (RISC-V trap instruction)
       ↓ the hardware automatically:
       ↓   - saves the current PC in sepc
       ↓   - switches the mode bit to kernel mode
       ↓   - jumps to the trap handler address set in stvec
[4] enters the kernel trap handler usertrap()
       ↓
[5] syscall(): reads a7 and looks up the system call table
       ↓ e.g., a7 = 15 (SYS_open)
[6] runs the sys_open() handler
       ↓ allocates a file descriptor, looks up the inode, etc.
[7] puts the result in a0 (e.g., fd = 3)
       ↓
[8] executes the sret instruction (RISC-V return)
       ↓ the hardware automatically:
       ↓   - restores the mode bit to user mode
       ↓   - restores the PC from sepc
[9] the C library wrapper passes a0 as the return value
       ↓
[10] the user program receives fd
```

*Figure 2. Full path of an open() system call on RISC-V*

#### 1.5.3 What Makes the ecall Instruction Special

⚠️ **Common confusion**: `ecall` is not an ordinary C function. It is a **special CPU-level instruction** that tells the hardware to:
1. Stop executing user code
2. Immediately transfer control to the OS kernel
3. Set the mode bit to kernel mode

This switch happens **atomically**, and the user program has no way to intervene in the middle.

#### 1.5.4 System Call Numbers and Argument Passing

Each system call is identified by a unique number.

| Architecture | Number register | Argument registers | Return value register |
|---|---|---|---|
| **RISC-V** | `a7` | `a0 ~ a5` (up to 6) | `a0` |
| **x86-64** | `rax` | `rdi, rsi, rdx, r10, r8, r9` | `rax` |

*Table 4. System call number, argument, and return value registers by architecture*

**System call table (syscall table)**: An array inside the kernel. The number is used as an index to find the pointer to the corresponding handler function.

```c
// Simplified model of xv6 kernel/syscall.c
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
    int num = myproc()->trapframe->a7;  // system call number
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        myproc()->trapframe->a0 = syscalls[num]();  // result goes in a0
    } else {
        myproc()->trapframe->a0 = -1;  // unknown syscall
    }
}
```

#### 1.5.5 libc Wrapper vs System Call

⚠️ **Common confusion**: `printf()` is **not** a system call but a C library function. `write()` is the actual system call.

Chain of calls:
1. `printf("Hello\n")`: user mode, C library
2. Buffering is performed internally
3. When the buffer fills or a newline arrives, the `write(1, buf, len)` system call is invoked
4. Enter the kernel and actually write to the file/console
5. Return to user mode

#### 1.5.6 Performance: System Calls Are Expensive

🧪 **Example**: Inside `cp in.txt out.txt`
- `open("in.txt", O_RDONLY)`: once
- `open("out.txt", O_WRONLY|O_CREAT)`: once
- Repeated in a loop:
  - `read(fd_in, buffer, 4096)`: read
  - `write(fd_out, buffer, bytes)`: write
- `close()` × 2

If the file is 4MB, that is about **1000 reads + 1000 writes = 2000 system calls**. Each call incurs mode-switch overhead (hundreds of ns to µs).

🔗 **Connection**: This is why optimizations such as **buffering**, **DMA**, **memory-mapped files (mmap)**, and **sendfile** matter. mmap, which builds on virtual memory, cuts this cost dramatically.

**One-Line Summary**: Because a user program cannot use privileged instructions directly, it raises a trap through a system call = ecall to have the kernel do the work on its behalf. Every kernel entry goes through this single passage.

### 1.6 How a Computer System Operates: Interrupts and DMA

#### 1.6.1 Bus-Based Architecture

```text
┌─────┐      Bus             ┌──────┐
│ CPU │ ←───────────────→  │Memory│
└─────┘                     └──────┘
   ↑
   ├─── Disk controller
   ├─── USB controller
   └─── Network controller
```

*Figure 3. Organization of a bus-based computer system*

**Bus**: The data communication path among the CPU, memory, and I/O devices.

#### 1.6.2 Interrupt

**Definition**: An asynchronous signal by which an external device notifies the CPU of **work completion or a service request**.

**Why it is needed**:
- The alternative, **polling**, is inefficient: the CPU must check device status periodically
- An interrupt wakes the CPU only when the device is ready → saves CPU resources

**Interrupt handling flow**:
1. When a device finishes its work, it sends a signal over the **interrupt line**
2. The CPU checks for the interrupt after finishing the current instruction
3. Look up the **interrupt vector table (IVT)**: it maps each interrupt number to the address of its handling routine
4. Run the corresponding **interrupt service routine (ISR)**
5. Return to the previous execution after handling

**Properties of interrupts**:
- **Asynchronous**: can occur at any time regardless of the instruction currently executing
- **Enables parallelism**: I/O devices work independently while the CPU does other work

#### 1.6.3 DMA (Direct Memory Access)

**Definition**: A mechanism that transfers data directly between memory ↔ I/O devices without CPU involvement.

**Why it is needed**:
- Without DMA, the CPU must move data **byte by byte** → reading a 1GB file requires billions of CPU instructions
- With DMA, the CPU only specifies the source address, destination address, and number of bytes, and can do other work

**DMA procedure**:
1. CPU → issues a command to the DMA controller (source address, destination address, byte count)
2. The CPU does other work
3. The DMA controller performs the data transfer (no CPU involvement)
4. When the transfer completes, the DMA controller **raises an interrupt**
5. The CPU runs the ISR to handle completion

🔗 **Connection**: High-performance I/O in modern systems (SSD, network card, GPU) is all DMA-based.

### 1.7 Storage Hierarchy

#### 1.7.1 The Speed-Capacity-Cost Triangle

| Level | Device | Capacity | Access time | Managed by |
|---|---|---|---|---|
| 0 | Registers | < 1 KB | about 0.3 ns | Hardware |
| 1 | L1/L2 cache | < 64 MB | about 1-25 ns | Hardware |
| 2 | Main memory (RAM) | < 64 GB | about 100 ns | **OS** |
| 3 | SSD | < 4 TB | about 50 µs | **OS** |
| 4 | HDD | < 20 TB | about 5 ms | **OS** |

*Table 5. Characteristics of each level of the storage hierarchy*

- **Toward the top**: faster, more expensive, and smaller
- **Toward the bottom**: slower, cheaper, and larger

#### 1.7.2 Principle of Locality: Why Caches Work

If a program's memory accesses were completely random, no cache would be effective. Fortunately, real programs show two kinds of **locality**.

🎯 **Temporal locality**: Data accessed recently is likely to be accessed again soon.
- Example: in `for (i=0; i<100; i++) sum += i`, `sum` is accessed 100 times in a row

🎯 **Spatial locality**: Data near recently accessed data is likely to be accessed soon.
- Example: accessing the array elements `a[0]`, `a[1]`, `a[2]`, ... sequentially

💡 **Book-reading analogy**: After reading page 50, you are likely to read page 51 next (spatial), and you may look at page 50 again (temporal). A cache exploits this predictability.

🔗 **Connection**: This principle is the premise of every caching technique, including the **TLB (address translation cache), page cache (file system cache), and buffer cache**.

### 1.8 OS Structure: How to Design the Kernel

There are four representative paradigms for organizing the OS kernel.

#### 1.8.1 Monolithic

Includes every OS function (process management, memory management, file system, drivers, etc.) in **a single kernel binary**.

- **Advantage**: Fast, because communication between functions is a **function call**. All the code can be optimized globally.
- **Disadvantage**: **A single bug brings down the whole system** (a file system bug → affects every process). Millions of lines of kernel code → hard to debug.
- **Examples**: Linux, traditional UNIX, Solaris

#### 1.8.2 Microkernel

**Keeps only a minimal kernel in kernel space** (IPC, scheduling, basic memory management). Most OS services (file system server, network stack, drivers) run as **server processes in user space**.

- **Advantages**:
  - **Fault isolation**: even if the file system server crashes, the kernel is safe and only a restart is needed
  - **Easy verification**: the kernel is small enough for formal verification
  - **Portability**: minimal platform dependence
- **Disadvantages**:
  - **Context switching overhead**: even communication between services requires IPC
  - **Implementation complexity**: every function must be separated into an independent process and coordinated
- **Examples**: Mach, QNX, L4

💡 **Why the microkernel did not become mainstream despite its theoretical superiority**: The answer is **performance**. The cost of user↔kernel switches piles up with every communication between services, which slows the system down.

#### 1.8.3 Hybrid

**Combines the advantages** of monolithic and microkernel designs. Performance-critical parts go in the kernel, and stability-critical parts go to user space.

- **Examples**: macOS (Mach microkernel + BSD Unix), Windows

#### 1.8.4 Loadable Modules

Only **the core** of the kernel is compiled statically, and the rest is loaded/unloaded as **dynamic modules**.

- **Examples**: Linux LKM (Loadable Kernel Module), `insmod`, `rmmod`

**In practice**: Most modern OSes use a **hybrid** + **loadable module** structure as a practical compromise.

### 1.9 Context Switching

**Definition**: Saving the state of the currently running process and restoring the state of another process to resume its execution.

#### 1.9.1 What the Context Includes

1. **CPU register values**: general-purpose registers, status register
2. **Program counter (PC)**: address of the next instruction to execute
3. **Stack pointer (SP)**
4. **Page table**: memory mapping information
5. **File descriptor table**
6. **Kernel metadata**: process state, priority, etc.

#### 1.9.2 Procedure

```text
Current process P1
  │
  ├─ [1] Save P1's registers, PC, SP, etc. to memory (P1's PCB)
  │
  ├─ [2] Read P2's state from memory and restore it to CPU registers (P2's PCB)
  │
  └─ [3] Set the PC to the point where P2 last stopped
       ↓
     P2 resumes execution
```

*Figure 4. Procedure of a context switch*

#### 1.9.3 Overhead and Optimization

**Cost factors**:
- Saving/restoring registers (dozens of registers)
- Cache invalidation (the new process's data may not be in the cache)
- TLB flush (clearing the virtual-to-physical address translation cache)

**Hardware optimizations**:
- **Multiple register sets** (SPARC register windows): no need to save registers to memory
- **Tagged TLB (ASID)**: tags TLB entries with an address space ID → no full TLB flush when switching address spaces

🔗 **Connection**: The fundamental reason microkernels are slow is that context switches occur frequently. Switching between threads is faster than switching between processes for the same reason: **no TLB flush is needed** (detailed in Chapter 4).

### 1.10 xv6: An Educational Operating System

**xv6** is a **Unix-like educational OS** built at MIT. In this course, xv6 is read and modified throughout the semester.

| Property | Description |
|---|---|
| Architecture | RISC-V |
| Language | C |
| Lines of code | **About 10,000 lines** |
| Scope | Processes, virtual memory, file system, shell |

*Table 6. Characteristics of xv6*

**Why xv6 is good teaching material**:
- Real Linux has tens of millions of lines → impossible to read
- xv6 has 10,000 lines → it can be read in full while still covering all the core OS concepts

**How to run it**:
```bash
git clone https://github.com/mit-pdos/xv6-riscv
cd xv6-riscv
make qemu    # boot xv6 on the QEMU emulator
```

**Required tools**:
- **RISC-V cross compiler** (`riscv64-unknown-elf-gcc`): produces RISC-V binaries on an x86/ARM host
- **QEMU** (`qemu-system-riscv64`): emulates RISC-V hardware

---

### Chapter 1 Wrap-Up 🎯

**Big Picture of Chapter 1**: OS = kernel. The kernel manages hardware resources and provides services to application programs. It protects itself with hardware-level dual mode and offers user programs a legitimate passage through system calls = traps. Interrupts and DMA make I/O efficient, and the storage hierarchy works because of the principle of locality.

**Likely Exam Questions**:
1. "What is an OS": a kernel-centered definition
2. "What if there were no dual mode": security vulnerabilities
3. "The full path of a system call": a step-by-step description
4. "The difference between a trap and an interrupt": synchronous/asynchronous
5. "Monolithic vs microkernel trade-off"

---

## Chapter 2. The Birth and Life of a Process (Week 2)

### 2.1 Process ≠ Program: A Fundamental Distinction 🎯

This distinction is a **conceptual foundation** that appears very often on exams.

| Program | Process |
|---|---|
| A **static file** stored on disk | An **entity in execution**, loaded into memory |
| Passive entity | Active entity |
| `.exe`, `a.out`, `.class` | Holds a PC, registers, memory, and state |
| One program | Can run concurrently as several processes |

*Table 7. Comparison of a program and a process*

🧪 **Real example**: Three users each run the **Chrome browser program (one copy)** installed once → **three processes**. The text (code) region of each process is identical, but the data, heap, and stack are independent for each.

#### 2.1.1 Why the Distinction Matters

- The same program can run as **several processes** → the basis of isolation
- A process itself can become **the execution environment for other code** (the JVM is an example)
- **Multiplexing**: several processes take turns on one CPU quickly, creating the illusion that they run at the same time

### 2.2 Memory Layout of a Process 🎯

A process is laid out in memory with the following structure.

```text
[↑ High address]
┌──────────────────────┐
│   Stack              │ ← grows down (on function calls)
│   [locals, args, return]│
├──────────────────────┤
│      ↕ (free space)  │
├──────────────────────┤
│   Heap               │ ← grows up (on malloc, new)
│   [dynamic allocation]│
├──────────────────────┤
│   BSS                │ ← uninitialized globals (auto-initialized to 0)
├──────────────────────┤
│   Data               │ ← initialized globals
├──────────────────────┤
│   Text               │ ← executable code (read-only)
└──────────────────────┘
[↓ Low address]
```

*Figure 5. Memory layout of a process*

#### 2.2.1 Details of Each Region

| Region | Contents | Size | Address growth |
|---|---|---|---|
| **Text (code)** | Machine instructions | Fixed, read-only | None |
| **Data** | Initialized global variables (`int x = 5;`) | Fixed | None |
| **BSS** | Uninitialized global variables (`int y;`) | Fixed, auto-initialized to 0 | None |
| **Heap** | Dynamic memory allocated with `malloc()`, `new` | Dynamic | ↑ Upward |
| **Stack** | Function call frames (local variables, parameters, return address) | Dynamic | ↓ Downward |

*Table 8. Contents and growth direction of each process memory region*

#### 2.2.2 Why It Is Designed This Way

🎯 **Why the stack and heap grow toward each other**: Both regions use one block of free space from opposite ends as much as they need → better memory efficiency. The OS watches that the two regions do not collide.

🎯 **Why BSS is separated from Data (important!)**:
- Initialized variables must **store their initial values in the executable file** (`int x = 1000000` → `sizeof(int)` bytes, usually 4, written to the file)
- Uninitialized variables only need to store **the size of the BSS region** (8 bytes in the example in Section 2.2.4)
- The OS automatically initializes the BSS region to 0 when loading the program
- **Result: the executable file becomes much smaller**

💡 **Origin of the name Text**: It comes from the `.text` directive of early assembly languages. It is a metaphor for the body (text) of the program.

#### 2.2.3 Stack Frame (Activation Record)

Each time a function is called, a **stack frame (activation record)** is pushed onto the stack.

Contents of a stack frame:
- The function's **local variables**
- **Parameters**
- **Return address** (the PC to return to after the function ends)
- **Previous stack frame pointer** (for restoring the caller)

**LIFO rule**: If A calls B and B calls C, the return order is C → B → A. The stack pointer (SP) moves from high addresses toward low addresses. This naturally supports recursive and nested calls.

#### 2.2.4 Analysis of a Real C Program

```c
#include <stdio.h>
#include <stdlib.h>

int x;           // uninitialized → BSS
int y = 15;      // initialized → Data

int main(int argc, char *argv[]) {
    int *values; // local variable → Stack
    int i;       // local variable → Stack

    values = (int *)malloc(sizeof(int) * 5);  // dynamic allocation → Heap
    for (i = 0; i < 5; i++)
        values[i] = i;
    return 0;
}
```

Running `size ./program` after compiling:
```text
text   data   bss   dec   hex
1158   284    8     1450  5aa
```

- `text`: size of the executable code section
- `data`: initialized global data
- `bss`: uninitialized global data

🔗 **Connection**: This memory layout is directly related to the page table in **virtual memory (Weeks 11-12)**. Each region is mapped to different pages, and the code region gets read-only page permissions.

### 2.3 Process States and State Transitions 🎯

During execution, a process is in one of the following five states.

```text
                      ┌───── dispatch ──────┐
                      │                     ↓
┌─────┐ admitted ┌─────────┐ interrupt ┌─────────┐   exit   ┌────────────┐
│ New │─────────→│  Ready  │←──────────│ Running │─────────→│ Terminated │
└─────┘          └─────────┘           └─────────┘          └────────────┘
                      ↑                     │
                      │ I/O or event        │ I/O or
                      │ completion          │ event wait
                      │   ┌─────────┐       │
                      └───│ Waiting │←──────┘
                          └─────────┘
```

*Figure 6. Process state transition diagram*

#### 2.3.1 The Five States

| State | Description | Condition |
|---|---|---|
| **New** | Being created | The OS has not yet loaded it into memory |
| **Ready** | Waiting for the CPU | Ready to run, waiting to be chosen by the scheduler |
| **Running** | Executing instructions | Actively running on a CPU core |
| **Waiting** | Waiting for an event | Waiting for I/O completion, a signal, etc. |
| **Terminated** | Finished | Waiting to be removed from the process table |

*Table 9. Five process states*

**Important principles**:
- On a **single core**, there is only **one Running process** at any moment
- Several processes can be Ready or Waiting at the same time
- On a **multicore** system, as many processes as there are cores can be Running

#### 2.3.2 Causes of State Transitions: A Frequent Exam Point

| Transition | Cause | Who decides |
|---|---|---|
| **New → Ready** | `admitted` (the OS approves creation and allocates memory) | OS |
| **Ready → Running** | `scheduler dispatch` (the scheduler chooses it) | Scheduler |
| **Running → Ready** | `interrupt` (**involuntary**, e.g., timer) | Timer hardware |
| **Running → Waiting** | `I/O or event wait` (**voluntary**) | The process itself |
| **Waiting → Ready** | `I/O or event completion` | I/O controller, ISR |
| **Running → Terminated** | `exit()` | The process itself |

*Table 10. Causes of state transitions and who decides them*

🎯 **A distinction that is often tested**:
- **Running → Ready (involuntary)**: the time slice expires → the OS forcibly takes back the CPU
- **Running → Waiting (voluntary)**: the process requests I/O → it gives up the CPU by itself

#### 2.3.3 Time Slice

**Time slice = quantum**: The fixed CPU time the OS allocates to each process (typically 10-100ms). When the quantum expires, a timer interrupt causes a Running → Ready transition.

🔗 **Connection**: The key parameter of round-robin (RR) scheduling (see Chapter 6).

### 2.4 Process Control Block (PCB) 🎯

**PCB (Process Control Block)** = the **data structure** the kernel maintains for each process. Its other name is **TCB (Task Control Block)**. It holds all the information needed to **start or restart** the process.

#### 2.4.1 Main Fields of the PCB

| Field | Description |
|---|---|
| **Process state** | new, ready, running, waiting, terminated |
| **Program counter (PC)** | Address of the next instruction to execute |
| **CPU registers** | Accumulator, index registers, SP, general-purpose, condition codes |
| **CPU scheduling information** | Priority, scheduling queue pointers |
| **Memory management information** | base/limit register values, page table |
| **Accounting information** | PID, CPU time used, process number |
| **I/O status information** | Allocated I/O devices, **list of open files** (fd table) |

*Table 11. Main fields of the PCB*

#### 2.4.2 The Most Important Fields

- **Program counter (PC)**: determines where to resume after a context switch
- **Process state**: determines whether the process can run

#### 2.4.3 Base/Limit Registers and Memory Protection

**Base register**: the **start address** of the memory the process can access
**Limit register**: the **size (range)** of that process's memory

**Hardware-enforced memory protection**: On every memory access, the CPU automatically checks whether the address lies within `[base, base+limit)`. Access outside the range → a trap occurs → the OS terminates the process.

🔗 **Connection**: This simple scheme evolves into the page table of virtual memory in Weeks 11-12.

#### 2.4.4 The PCB and Context Switches

When a context switch occurs:
1. **Save the current process state**: PC, all registers, memory management information → saved in the current process's PCB
2. **Restore the new process state**: PC and register values from the new process's PCB → loaded into the CPU

⚠️ **Context switching is impossible without the PCB!** A suspended process could not be resumed exactly later.

#### 2.4.5 Linux task_struct

Linux implements the PCB as the `task_struct` structure. Location: `<include/linux/sched.h>`.

```c
struct task_struct {
    long state;                    // process state
    struct sched_entity se;        // scheduling information
    struct task_struct *parent;    // pointer to the parent process
    struct list_head children;     // list of child processes (doubly linked)
    struct files_struct *files;    // open files (fd table)
    struct mm_struct *mm;          // address space (memory information)
    // ... hundreds more ...
};
```

- All active processes are managed in a **doubly linked list** (O(1) insertion/deletion)
- `current`: pointer to the `task_struct` of the currently running process

### 2.5 Overview of Process Scheduling

#### 2.5.1 Why Scheduling Is Needed

**Multiprogramming**: Keeps some process running at all times to **maximize CPU utilization**. While the CPU waits for I/O to complete, another process runs.

**Time sharing**: Switches the CPU frequently among processes to give users **interactivity**.

#### 2.5.2 I/O-Bound vs CPU-Bound

| Type | Characteristic | Examples |
|---|---|---|
| **I/O-bound** | Spends more time on I/O than on computation | Web browser, DB app |
| **CPU-bound** | Spends a long time on computation | Scientific computing, image processing |

*Table 12. Comparison of I/O-bound and CPU-bound processes*

**A proper mix** is important:
- Only I/O-bound processes → processes often wait for I/O → the CPU is idle
- Only CPU-bound processes → the devices are idle
- With a mix, both the CPU and the devices are highly utilized

#### 2.5.3 Scheduling Queues

**Ready queue**: A linked list of all processes waiting for the CPU.

**Wait queue (device queue)**: A queue of processes waiting for a specific event. There are several, one per device.

```text
New process
     ↓ memory allocated
 [Ready Queue] ────→ [CPU scheduler] ────→ Running
     ↑                                    ↓
     │                              time slice expired
     │                                    │
     │                              I/O request
     └─ I/O done  ←── [Device Queue] ←────┘
                     (disk, terminal, etc.)
```

*Figure 7. Movement of processes between the ready queue and device queues*

### 2.6 fork(): Creating a Process 🎯

#### 2.6.1 Parent-Child Relationship and the Process Tree

**Parent process**: A process that creates another process.
**Child process**: A newly created process.
**Process tree**: The tree formed by parent-child relationships.

In Linux, **systemd** (PID=1) is the **root parent** of all user processes. The kernel creates it at boot.

```text
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

*Figure 8. Example process tree rooted at systemd*

#### 2.6.2 How fork() Works

**Definition**: `fork()` is a system call that creates a child process by **completely duplicating** the address space of the calling process.

```text
┌──────────────────┐       ┌──────────────────┐       ┌──────────────────┐
│  Parent (PID 1000)│ fork()│  Parent (PID 1000)│      │  Child (PID 1001)│
│  Text            │  ───→  │  Text            │  +    │  Text            │
│  Data: x=5       │        │  Data: x=5       │       │  Data: x=5       │
│  Heap            │        │  Heap            │       │  Heap            │
│  Stack           │        │  Stack           │       │  Stack           │
└──────────────────┘       └──────────────────┘       └──────────────────┘
                                                          ↑ complete copy
```

*Figure 9. Parent and child address spaces before and after fork()*

#### 2.6.3 The fork() Return Value Convention ⚠️

```c
pid_t pid = fork();
```

| pid value | Meaning |
|---|---|
| **0** | Child process (the newly created one) |
| **> 0** | Parent process (the value is the child's PID) |
| **-1** | fork() failed |

*Table 13. Meaning of the fork() return value*

**Why it is designed this way**:
- **Why the parent receives the child's PID**: the parent needs the PID to manage the child (wait, kill)
- **Why the child receives 0**: the child can learn its own PID with `getpid()` and its parent's PID with `getppid()`. 0 is the simplest protocol.

#### 2.6.4 COW (Copy-On-Write) Optimization

⚠️ **Problem**: If fork() **copies memory completely**, it is expensive. Moreover, if the child calls exec() right away, all the copied memory is thrown away → **extreme waste**.

🎯 **Solution**: **COW (Copy-On-Write)**
- Right after fork(), the parent and child **share the same physical memory pages**
- The page table flags are marked read-only
- **When either side tries to write**, the page is copied at that moment and the flag is changed to read/write
- Most pages are never actually copied

🔗 **Connection**: This optimization brings the cost of fork() close to zero and is decisive especially in the **fork + exec pattern**.

### 2.7 exec(): Replacing the Process Image 🎯

**Definition**: **Completely replaces** the memory image of the current process **with a new program**.

#### 2.7.1 Properties of exec()

- **The PID does not change**: same process, different program
- **Does not return on success**: the memory is overwritten, so there is no way back to the previous code
- **Returns only on failure** (-1): wrong path, insufficient permissions, etc.

```text
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
PID = 1234                   PID = 1234 (unchanged!)
```

*Figure 10. Memory image before and after exec()*

#### 2.7.2 The exec() Family of Functions

The C library provides several variants. Naming rules:
- **l** (list): variable-length argument list
- **v** (vector): array of strings
- **p** (path): searches the PATH environment variable
- **e** (environment): specifies a custom environment

| Function | Argument format | Path | Environment |
|---|---|---|---|
| `execl(path, arg0, arg1, ..., NULL)` | Variable list | Full path | Inherited |
| `execlp(file, arg0, arg1, ..., NULL)` | Variable list | **PATH search** | Inherited |
| `execv(path, argv[])` | Array | Full path | Inherited |
| `execvp(file, argv[])` | Array | **PATH search** | Inherited |

*Table 14. Argument format, path, and environment of the exec() family*

🧪 **The most commonly used form**:
```c
execlp("ls", "ls", "-l", NULL);
// first "ls": name of the program to search for
// second "ls": argv[0], the program name by UNIX convention
// "-l": argv[1]
// NULL: marks the end of the argv array
```

### 2.8 The fork + exec + wait Pattern: A UNIX Masterpiece 🎯

#### 2.8.1 Basic Pattern

```c
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        // error
        fprintf(stderr, "fork failed\n");
        return 1;
    } else if (pid == 0) {
        // child process
        printf("Child: pid=%d, parent=%d\n", getpid(), getppid());
        execlp("/bin/ls", "ls", "-l", NULL);
        // never reached if exec succeeds
        perror("exec failed");
        return 1;
    } else {
        // parent process
        printf("Parent: my pid=%d, child pid=%d\n", getpid(), pid);
        wait(NULL);  // wait for the child to terminate
        printf("Child finished\n");
    }
    return 0;
}
```

#### 2.8.2 The Brilliant Design of UNIX: Why fork and exec Are Separate

🎯 **Key insight**: Separating `fork()` and `exec()` creates an opportunity to customize the child's environment after the child process is created but **before it runs** the new program.

Possible customizations:
- **File descriptor redirection**: connect stdin/stdout to a pipe or file with `dup2()` → implements the shell's `>`, `<`, `|`
- **Changing environment variables**: set an environment just for the child with `setenv()`
- **Changing the working directory**: `chdir()`
- **Changing the user/group**: `setuid()`, `setgid()` (dropping privileges)
- **Resetting signal handling**

#### 2.8.3 Contrast with Windows CreateProcess()

```c
// Windows style (one function specifies every option)
CreateProcess(
    NULL, "ls -l", NULL, NULL, FALSE,
    0, NULL, NULL, &startupInfo, &processInfo
);
```

Windows must specify every option at once at creation time, so it **is less flexible**.

| | UNIX fork+exec | Windows CreateProcess |
|---|---|---|
| Design philosophy | Combine small tools | One large integrated function |
| Number of parameters | fork: 0, exec: a few | 10 or more |
| Flexibility | High (free customization after fork) | Low |
| Complexity | Simple through combination | A single complex call |

*Table 15. Comparison of UNIX fork+exec and Windows CreateProcess*

#### 2.8.4 The Role of wait()

Three things **wait()** does:
1. **Waits for the child to terminate**: the parent blocks
2. **Collects the child's exit status**: prevents zombies
3. **Returns the PID of the terminated child**

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

| Macro | Purpose |
|---|---|
| `WIFEXITED(s)` | Terminated normally? |
| `WEXITSTATUS(s)` | Exit code (0-255) |
| `WIFSIGNALED(s)` | Terminated by a signal? |
| `WTERMSIG(s)` | Number of the signal that terminated it |

*Table 16. Macros for inspecting the wait() exit status*

### 2.9 Zombie and Orphan: A Frequently Tested Distinction 🎯

#### 2.9.1 Zombie Process

**State**: The child has **terminated (exit)**, but the parent has not yet called `wait()`, so the child **remains only in the process table**.

**Characteristics**:
- Execution is completely finished (uses no memory or CPU)
- Occupies only a process table entry (PID, exit code)
- **Every process passes briefly through the zombie state when it terminates** (normal)

**Problem**: If the parent never calls wait(), **zombies accumulate** → process table slots are wasted → no new processes can be created.

To observe:
```bash
ps aux | grep defunct
# or state "Z"
```

#### 2.9.2 Orphan Process

**State**: A child process **left behind** because its parent terminated first.

**Characteristics**:
- The child is **still running** (or waiting)
- Its parent is gone

**How Linux handles it**:
1. The kernel searches for all children of the parent
2. Each child's PPID (parent PID) → **changed to init/systemd (PID=1)**
3. When the child terminates, systemd periodically calls `wait()` to clean it up

#### 2.9.3 Comparison Table

| | **Zombie** | **Orphan** |
|---|---|---|
| Child state | Terminated (called exit) | Still running |
| Cause | Parent did not call wait() | Parent terminated first |
| Resources | Wastes only a process table slot | Running normally |
| Resolution | Parent calls wait() / adopted by init if the parent dies | Adopted automatically by init/systemd |
| Accumulation | Yes (dangerous) | No (handled automatically) |

*Table 17. Comparison of zombie and orphan processes*

#### 2.9.4 How to Remove Zombies

⚠️ **Important**: A zombie cannot be killed with `kill`, because it is already dead!

1. **Send SIGCHLD to the parent** → prompts the parent to call wait()
2. **Forcibly terminate the parent process** → the zombie's parent changes to init → init cleans it up automatically
3. **Prevention**: set `signal(SIGCHLD, SIG_IGN)` in POSIX → the OS reaps zombies automatically

Memory aids:
- Zombie: already dead, yet it stays in the process table instead of lying in its grave (like a zombie)
- Orphan: the parent is gone, so systemd adopts it (an orphanage)

### 2.10 Chrome's Multiprocess Architecture: A Real-World Case

Problems of a single-process browser:
- One tab crashes → the whole browser goes down
- A malicious script on one site → affects every tab

**Chrome's solution**: multiple processes

| Process | Role | Count |
|---|---|---|
| Browser | UI management, I/O, process monitoring | 1 |
| Renderer | Web page rendering (HTML/CSS/JS) | 1 per tab |
| Plug-in | Runs plug-ins | 1 per type |
| GPU | Graphics acceleration | 1 |

*Table 18. Process types in Chrome*

**Benefits**:
1. **Isolation**: even if tab 1 crashes, tabs 2 and 3 are fine
2. **Security**: renderers run in a **sandbox** (an isolated environment with restricted privileges)
3. **Responsiveness**: an infinite JS loop in one tab does not affect other tabs
4. **Parallelism**: makes use of multiple cores

🔗 **Connection**: This pattern is a representative architectural decision that improved performance, stability, and security at once by moving from a single process to multiple processes.

---

### Chapter 2 Wrap-Up 🎯

**Big Picture of Chapter 2**: A process is the embodiment of a program in execution and has its own memory layout (Text/Data/BSS/Heap/Stack) and state (New/Ready/Running/Waiting/Terminated). The PCB holds the process information and makes context switches possible. fork/exec/wait is the elegant process management mechanism of UNIX, and zombies and orphans are special states in the life cycle.

---

## Chapter 3. Inter-Process Communication (IPC) (Week 3)

### 3.1 Why IPC Is Needed

In reality, processes must **cooperate with each other**.

#### 3.1.1 Independent vs Cooperating Processes

- **Independent process**: no interaction with other processes
- **Cooperating process**: shares data and exchanges results

#### 3.1.2 Four Reasons Cooperation Is Needed

1. **Information sharing**: copy-and-paste
2. **Computation speedup**: parallelizing work
3. **Modularity**: separating functions into processes
4. **Convenience**: the user performs several tasks at once

#### 3.1.3 Real-World IPC Examples

| Application | IPC mechanism |
|---|---|
| Shell pipeline `cat log \| grep err` | Pipe |
| Google Chrome | Shared memory + IPC |
| Docker | Named pipe (FIFO), socket |
| PostgreSQL | Shared memory (buffer cache) |
| Slack/Discord | Socket (WebSocket) |
| Android apps | Binder (RPC) |

*Table 19. IPC mechanisms by application*

### 3.2 Two IPC Paradigms 🎯

#### 3.2.1 Shared Memory vs Message Passing

| | Shared memory | Message passing |
|---|---|---|
| Method | Read/write a shared region | send/receive |
| Speed | Fast (no kernel involvement) | Slow (a syscall on every call) |
| Synchronization | **Programmer's responsibility** | Managed by the OS |
| Suited for | Large data, local | Small data, distributed systems |
| Initial setup | `shm_open`, `mmap` | Only setting up a link |
| Recurring cost | 0 (like ordinary memory) | High (system calls) |

*Table 20. Comparison of shared memory and message passing*

#### 3.2.2 Mechanism of Shared Memory

Shared memory is implemented by the OS **manipulating the page tables of virtual memory**:
- The virtual addresses of different processes are set to point to **the same physical frame**
- After mapping, access is the same as ordinary memory access → very fast
- But synchronization must be done by the programs themselves

### 3.3 The Producer-Consumer Problem

#### 3.3.1 The Classic Pattern

**Producer**: generates data
**Consumer**: consumes data

Real examples:
- Compiler (produces) → assembler (consumes)
- Web server (produces) → browser (consumes)
- Keyboard (produces) → OS (consumes)

#### 3.3.2 Implementing a Bounded Buffer

```c
#define BUFFER_SIZE 10

typedef struct { /* ... */ } item;
item buffer[BUFFER_SIZE];
int in = 0;     // next write position
int out = 0;    // next read position
```

**Circular array** mechanism:
- When an index reaches BUFFER_SIZE, it wraps around to 0
- **Empty**: `in == out`
- **Full**: `((in + 1) % BUFFER_SIZE) == out`
- **Maximum stored**: `BUFFER_SIZE - 1` items

🎯 **Why one slot is left empty**: To **distinguish** empty (in==out) from full. If every slot were used, the two states would look the same.

**Producer code**:
```c
while (true) {
    // produce next_produced
    while (((in + 1) % BUFFER_SIZE) == out)
        ; // busy wait: buffer full

    buffer[in] = next_produced;
    in = (in + 1) % BUFFER_SIZE;
}
```

**Consumer code**:
```c
while (true) {
    while (in == out)
        ; // busy wait: buffer empty

    next_consumed = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    // consume next_consumed
}
```

#### 3.3.3 The Problem with Busy Waiting

⚠️ **Busy waiting**: keeps spinning in a while loop checking the condition → wastes the CPU.

🔗 **Connection**: Week 9 introduces blocking approaches with **semaphores, mutexes, and condition variables**. A waiting process is put to sleep, freeing the CPU.

### 3.4 Three Design Axes of Message Passing

#### 3.4.1 Naming: Direct vs Indirect

**Direct communication**:
- `send(P, message)` / `receive(Q, message)`: the peer is named explicitly
- **Symmetric**: both sides name the peer (fixed 1:1)
- **Asymmetric**: the receiver receives from any process (`receive(id, msg)`), the server pattern
- Property: changing a process identifier requires changing every reference → **limited modularity**

**Indirect communication**:
- Through a **mailbox / port**
- `send(A, msg)` / `receive(A, msg)`
- Two processes share a common mailbox → flexible in dynamic environments
- One link can be associated with **more than two** processes

#### 3.4.2 Synchronization: Blocking vs Nonblocking

| | send | receive |
|---|---|---|
| Blocking (synchronous) | Waits until the receiver receives it | Waits until a message arrives |
| Nonblocking (asynchronous) | Sends and continues immediately | Receives if possible, otherwise null |

*Table 21. Behavior of blocking and nonblocking send and receive*

**Rendezvous**: With blocking send + blocking receive, both sides wait for each other. The producer-consumer problem becomes very simple.

#### 3.4.3 Buffering

| Buffering | Capacity | Sender behavior |
|---|---|---|
| **Zero capacity** | No waiting messages | **Must wait** until the receiver receives it |
| **Bounded** | At most n | Waits when full |
| **Unbounded** | No limit | Never waits (unrealistic) |

*Table 22. Capacity and sender behavior by buffering scheme*

### 3.5 POSIX Shared Memory: Practical Code

#### 3.5.1 Three-Step API

```c
// Step 1: create the shared memory object
int fd = shm_open(name, O_CREAT | O_RDWR, 0666);

// Step 2: set its size
ftruncate(fd, 4096);

// Step 3: map it into the address space
char *ptr = (char *)mmap(0, 4096, PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0);
```

#### 3.5.2 Details of Each Function

- **`shm_open(name, flags, mode)`**: creates/opens a shared memory object
  - `name`: processes access it by the same name
  - `O_CREAT | O_RDWR`: create if it does not exist, read/write
  - `0666`: file permissions (rw-rw-rw-)

- **`ftruncate(fd, SIZE)`**: sets the size. Required because the default length of new shared memory is 0.

- **`mmap(addr, length, prot, flags, fd, offset)`**: mapping
  - `prot`: `PROT_READ | PROT_WRITE`
  - `flags`: `MAP_SHARED` (changes are visible to other processes) vs `MAP_PRIVATE`

#### 3.5.3 Complete Example: Producer/Consumer

**Producer**:
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

**Consumer**:
```c
int main() {
    const int SIZE = 4096;
    const char *name = "OS";

    int fd = shm_open(name, O_RDONLY, 0666);
    char *ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);

    printf("%s", ptr);  // prints "HelloWorld!"

    shm_unlink(name);
    return 0;
}
```

Compiling:
```bash
gcc producer.c -o producer -lrt  # -lrt: POSIX real-time library
```

#### 3.5.4 Practical Uses

| System | How it is used |
|---|---|
| PostgreSQL | Buffer pool shared among connections (8KB page cache) |
| Redis | fork-based persistence: parent↔child COW sharing |
| Chromium | Renderer ↔ GPU texture sharing |
| Video editors | Decoder ↔ player frame sharing |

*Table 23. Practical uses of shared memory*

### 3.6 Pipes: The Essence of the UNIX Philosophy

#### 3.6.1 Ordinary Pipe

**Definition**: A unidirectional data channel between two processes. It is a special file accessed with `read()/write()`.

```c
int fd[2];
pipe(fd);
// fd[0] = read end (READ_END)
// fd[1] = write end (WRITE_END)
```

**Properties**:
- **Unidirectional** (two-way communication needs two pipes)
- The child inherits the pipe through **fork()**
- **Requires a parent-child relationship**

#### 3.6.2 Practical Example

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
        // parent: write
        close(fd[READ_END]);
        write(fd[WRITE_END], write_msg, strlen(write_msg) + 1);
        close(fd[WRITE_END]);
    } else if (pid == 0) {
        // child: read
        close(fd[WRITE_END]);
        read(fd[READ_END], read_msg, BUFFER_SIZE);
        printf("read %s\n", read_msg);
        close(fd[READ_END]);
    }

    return 0;
}
```

#### 3.6.3 Essential Pipe Rules 🎯

⚠️ **The most important rule**: Unused ends **must be closed**.

**Reason**: EOF occurs on the read side only **when every write end is closed**. If the ends are not closed, the read side **blocks forever**.

**Mechanism in detail**:
1. After `fork()`, both the parent and the child have copies of `fd[0]` and `fd[1]`
2. If the parent does not close `fd[READ_END]`, which it will not use, the kernel judges that a read fd is still open
3. The child (the read side) is also in a state where it can write to the write end `fd[WRITE_END]`
4. In the end the kernel sees that a writable fd exists and does not deliver EOF
5. read **blocks permanently**

⚠️ **Meaning of `strlen() + 1`**: The `+1` includes the null terminator `\0`. If it is left out, `printf("%s")` does not know where the string ends.

#### 3.6.4 Shell Pipeline: Implementing `ls | wc -l`

```c
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pipe(fd);

    // first child: ls
    if (fork() == 0) {
        close(fd[0]);  // close the read end
        dup2(fd[1], STDOUT_FILENO);  // stdout → pipe
        close(fd[1]);
        execlp("ls", "ls", NULL);
        perror("ls");
        return 1;
    }

    // second child: wc
    if (fork() == 0) {
        close(fd[1]);  // close the write end
        dup2(fd[0], STDIN_FILENO);   // stdin → pipe
        close(fd[0]);
        execlp("wc", "wc", "-l", NULL);
        perror("wc");
        return 1;
    }

    // parent: must close both fds! (important!)
    close(fd[0]);
    close(fd[1]);

    wait(NULL);
    wait(NULL);
    return 0;
}
```

🎯 **Key point**: The parent does not use the pipe either, so it **closes both ends**. Otherwise wc never receives EOF.

#### 3.6.5 Named Pipe (FIFO)

An advanced pipe that overcomes the limitation of ordinary pipes (the need for a parent-child relationship).

| | Ordinary pipe | Named pipe (FIFO) |
|---|---|---|
| Process relationship | Parent-child | **Unrelated processes also possible** |
| Lifetime | Destroyed when the processes terminate | Persists in the file system |
| Identification | fd number | **File system path** |

*Table 24. Comparison of ordinary pipes and named pipes*

```c
// create
mkfifo("/tmp/my_fifo", 0666);

// writing process
int fd = open("/tmp/my_fifo", O_WRONLY);
write(fd, "Hello", 6);
close(fd);

// reading process (a separate program)
int fd = open("/tmp/my_fifo", O_RDONLY);
read(fd, buf, 6);
close(fd);
```

| OS | Characteristics |
|---|---|
| UNIX | Half-duplex (one direction at a time) |
| Windows | Full-duplex, remote machines possible |

*Table 25. Characteristics of named pipes by OS*

#### 3.6.6 dup2(): I/O Redirection

```c
int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
dup2(fd, STDOUT_FILENO);  // redirect stdout to output.txt
close(fd);

printf("Redirected!\n");  // written to output.txt
```

**dup2(oldfd, newfd)**: Makes newfd point to the same file as oldfd.

**Standard fds**:
- `STDIN_FILENO` = 0 (keyboard)
- `STDOUT_FILENO` = 1 (screen)
- `STDERR_FILENO` = 2 (screen, errors)

**How the shell implements `ls > file.txt`**:
1. fork
2. Child: `open("file.txt", O_WRONLY|O_CREAT|O_TRUNC)` → fd
3. `dup2(fd, STDOUT_FILENO)` → stdout goes to file.txt
4. `close(fd)`
5. `execlp("ls", "ls", NULL)`: the output of ls goes to file.txt automatically

💡 **UNIX philosophy**: "Do one thing well, and connect with pipes." Thanks to this philosophy, powerful combinations such as `find / -name "*.log" | xargs grep ERROR | sort -u` are possible.

### 3.7 Socket

#### 3.7.1 Socket = Endpoint of Communication

**Definition**: An endpoint of network communication. Identified by **IP address + port number**.

**Well-known ports**:
| Port | Service |
|---|---|
| 22 | SSH |
| 80 | HTTP |
| 443 | HTTPS |
| 53 | DNS |
| 25 | SMTP |
| 21 | FTP |

*Table 26. Well-known port numbers*

**Loopback**: `127.0.0.1`, that is, the same machine itself.

#### 3.7.2 TCP vs UDP

| | TCP | UDP |
|---|---|---|
| Connection | Connection-oriented | Connectionless |
| Reliability | Guaranteed (ordering, retransmission) | None |
| Speed | Slow | Fast |
| Uses | HTTP, SSH, DB | Streaming, games, DNS |

*Table 27. Comparison of TCP and UDP*

💡 **Analogy**:
- TCP = a phone call (connect, then talk and confirm)
- UDP = a letter (sent right away, no delivery guarantee)

#### 3.7.3 Practical Socket Use

| App | Protocol | Use |
|---|---|---|
| ChatGPT | WebSocket (TCP) | Token streaming |
| Multiplayer games | UDP | Low-latency positions |
| Zoom | UDP (WebRTC) | Real-time audio/video |
| Web browser | TCP (HTTP/S) | Page loading |
| DB client | TCP or UNIX socket | psql, mysql |

*Table 28. Socket use by application*

#### 3.7.4 Limitations of Sockets

Sockets exchange only **unstructured byte streams**. Structuring the data is the app's responsibility → **serialization** is needed (JSON, Protocol Buffers).

### 3.8 RPC (Remote Procedure Call)

#### 3.8.1 Concept

**Definition**: A mechanism for **calling a procedure on a networked remote system as if it were a local function**.

#### 3.8.2 Components

- **Stub**: A client-side proxy. It looks like the real function, but internally it marshals the parameters and sends them over the network
- **Skeleton**: The server side. Unmarshals the received data → calls the real function

#### 3.8.3 Marshalling and Endianness

**Marshalling**: Converting data into a format suitable for network transmission.

**Endianness**: The storage order of multibyte data.

Example: `0x12345678`
- **Big endian**: `12 34 56 78` (network byte order)
- **Little endian**: `78 56 34 12` (x86, RISC-V)

**Problem**: If the client (little) and server (big) have different architectures, the data is corrupted. Solution: marshal into a neutral format such as **XDR (External Data Representation)**.

#### 3.8.4 Execution Flow

```text
Client                                    Server
──────                                ──────
1. Call foo(x, y)
      ↓
2. Client stub: marshal the parameters
      ↓
3. Send over the network
      ↓
4. Server skeleton: unmarshal
      ↓
5. Run the real foo()
      ↓
6. Marshal the result & send it back
      ↓
7. Client stub: unmarshal
      ↓
8. Provide the return value
```

*Figure 11. Execution flow of an RPC call*

#### 3.8.5 Execution Semantics: Very Important

Network packets can be lost → retransmission → risk of duplicate execution. The three semantics that handle this:

| Semantics | Description | Safe use |
|---|---|---|
| **At-least-once** | Retransmits until ACK, no duplicate check | **Idempotent operations only** (queries) |
| **At-most-once** | Ignores duplicates using timestamps | Some loss tolerated |
| **Exactly-once** | at-most-once + ACK | **Required for financial transactions** |

*Table 29. Comparison of RPC execution semantics*

Whether an operation is **idempotent** is the key:
- Idempotent: the result is the same no matter how many times it runs (balance inquiry)
- Non-idempotent: the number of executions affects the result (money transfer)

**Implementation**: A unique transaction ID. The server records the IDs it has received and returns the previous result for a duplicate ID.

#### 3.8.6 Modern RPC: gRPC

- **Marshalling**: Protocol Buffers (binary, compact)
- **Transport**: HTTP/2 (multiplexing, streaming)
- **Automatic stub generation**: supports 10+ languages
- Used by: Netflix, Uber, Kubernetes

```protobuf
service WeatherService {
  rpc GetForecast (Location) returns (Forecast);
}
```

The client calls `GetForecast()` as if it were local, and gRPC handles marshalling and transmission automatically.

### 3.9 Analyzing the xv6 Kernel: Lab

#### 3.9.1 struct proc: The PCB of xv6

```c
struct proc {
  struct spinlock lock;
  enum procstate state;        // UNUSED → USED → RUNNABLE → RUNNING → ZOMBIE
  void *chan;                  // sleep channel (SLEEPING state)
  int killed;                  // pending kill signal
  int xstate;                  // exit status passed to the parent
  int pid;                     // process ID

  struct proc *parent;         // parent pointer

  uint64 kstack;               // kernel stack virtual address
  uint64 sz;                   // process memory size
  pagetable_t pagetable;       // user page table
  struct trapframe *trapframe; // saved user registers
  struct context context;      // saved kernel registers
  struct file *ofile[NOFILE];  // open files
  struct inode *cwd;           // current working directory
  char name[16];               // process name
};
```

**Important fields**:
- **trapframe**: saves user registers on user ↔ kernel switches
- **context**: used for the scheduler's kernel ↔ kernel switches
- Why the two are separate: they handle different kinds of switches

#### 3.9.2 Steps of xv6 fork()

1. **allocproc()**: allocates a new slot, pid, kstack, and trapframe
2. **uvmcopy()**: copies the parent's page table + memory (**the most expensive step, done first**)
3. **Copy the trapframe**: so the child can also return from fork()
4. **Set a0 = 0**: makes the child's fork() return value 0
5. **Copy ofile[]**: inherits the open fds
6. **Set parent, state = RUNNABLE**
7. **Return the child PID to the parent**

🎯 **Meaning of step 4**: In RISC-V, `a0` is the function return value register. Setting the child's `trapframe->a0 = 0` makes the child read 0 from `a0` when it returns to user mode, so it recognizes that it is the child. The parent receives the child PID as usual. This is the secret of **the same code returning different values in two processes**.

---

### Chapter 3 Wrap-Up 🎯

**Big Picture of Chapter 3**: Processes are isolated but need to cooperate. Shared memory (fast but needs synchronization) and message passing (safe but slow) are the two paradigms. Pipes are the essence of the UNIX philosophy, and named pipes (FIFO) are based on the file system and connect even unrelated processes. Sockets extend this over the network, and RPC abstracts remote calls so they look like local functions.

---

## Chapter 4. Fundamentals of Threads and Concurrency (Week 4)

### 4.1 What Is a Thread

#### 4.1.1 Definition

**Thread** = the **basic unit of CPU utilization** within a process. If a process is the unit of resources, a thread is the unit of execution.

#### 4.1.2 What a Thread Owns and Shares

**Owned independently by each thread**:
| Resource | Role |
|---|---|
| Thread ID | Unique identifier |
| Program counter (PC) | Next instruction to execute |
| Register set | The thread's CPU state |
| Stack | Function calls, local variables |

*Table 30. Resources each thread owns independently*

**Shared by all threads of the same process**:
| Resource | Description |
|---|---|
| Code (Text) | Program code |
| Data | Global variables |
| Heap | Dynamic allocation |
| File descriptors | Open files |
| Signal handlers | |

*Table 31. Resources shared by the threads of a process*

#### 4.1.3 Single-Threaded vs Multithreaded

```text
Single-threaded:
┌─────────────┐
│ Code + Data │
├─────────────┤
│ PC Reg Stack│ ← one
└─────────────┘

Multithreaded:
┌─────────────────────────────────┐
│        Code + Data              │ ← shared
├─────────────────────────────────┤
│ PC₀ Reg₀ Stk₀ │ PC₁ Reg₁ Stk₁ │ ...
└─────────────────────────────────┘
    T0            T1
```

*Figure 12. Structure of single-threaded and multithreaded processes*

### 4.2 Detailed Comparison of Processes vs Threads 🎯

| | Process | Thread |
|---|---|---|
| **Creation cost** | High (memory allocation, page table) | Low (only a stack and registers) |
| **Context switch** | Slow (TLB flush) | **Fast (no TLB flush)** |
| **Memory sharing** | Isolated (IPC needed) | Shared naturally (global variables) |
| **Independence** | High (if one dies, the others survive) | Low (if one dies, the whole process is affected) |

*Table 32. Comparison of processes and threads*

#### 4.2.1 The TLB and the Speed Difference: Why It Matters

**TLB (Translation Lookaside Buffer)**: A cache (hardware) for virtual→physical address translation.

- **Switching between processes**: the page table changes → a TLB flush is needed → frequent new TLB misses
- **Switching between threads**: same page table → the TLB stays valid → no flush needed → fast

🔗 **Connection**: This difference is one of the biggest reasons to use threads.

### 4.3 Four Benefits of Multithreading 🎯

#### 4.3.1 Responsiveness

The UI thread responds to user input even while other work is blocking.
- Long-running work runs **asynchronously** in a separate thread
- Web browser: network thread + rendering thread + input thread

#### 4.3.2 Resource Sharing

Because the threads share one address space, **sharing is automatic**: they communicate directly through global variables without IPC.

#### 4.3.3 Economy

Creating and switching threads costs much less than processes.
- Web server: a thread per request (low cost) instead of a process per request (high cost)

#### 4.3.4 Scalability

**True parallel execution** is possible on multiprocessors/multicores.
- A single-threaded process = uses only one core regardless of the number of cores
- Multithreaded = each thread placed on a different core → higher throughput

### 4.4 Concurrency vs Parallelism 🎯

#### 4.4.1 Key Definitions

**Concurrency**: A state in which several tasks are **making progress**.
- Possible even on a single core (time-sharing interleaving)

**Parallelism**: Several tasks **actually execute at the same time**.
- Possible only on multicore systems

#### 4.4.2 Relationship

**Parallelism ⊂ concurrency**. All parallelism is concurrency, but not the other way around.

| | Single core | Multicore |
|---|---|---|
| Concurrency | O (interleaving) | O |
| Parallelism | **X** | O |

*Table 33. Whether concurrency and parallelism are possible by number of cores*

#### 4.4.3 Visualization

```text
Single core (time sharing):
Core: [T1][T2][T3][T4][T1][T2]...
      ↑ concurrency through interleaving, no parallelism

Multicore (parallel):
Core1: [T1    ][T3    ][T1    ]...
Core2: [T2    ][T4    ][T2    ]...
       ↑ T1 and T2 actually run at the same time
```

*Figure 13. Concurrency on a single core and parallelism on multiple cores*

💡 **Analogy**:
- Concurrency: one person alternates between cooking and doing laundry
- Parallelism: two people cook and do laundry separately

### 4.5 Challenges of Multicore Programming

1. **Identifying tasks**: finding independent, separable tasks
2. **Balance**: equal amounts of work on each core
3. **Data splitting**: minimizing conflicts
4. **Data dependency**: synchronization needed
5. **Testing and debugging**: nondeterminism, Heisenbugs

**Heisenbug**: A bug that disappears when observed with a debugger (because the timing changes).

### 4.6 Amdahl's Law 🎯

#### 4.6.1 Formula

$$ \text{speedup} \le \frac{1}{S + \frac{1-S}{N}} $$

- **S**: serial fraction (the part that cannot be parallelized)
- **N**: number of processing cores

#### 4.6.2 Meaning: The Limit of Infinite Cores

As N → ∞, **speedup → 1/S**.

💡 **Interpretation**: No matter how many cores are added, the serial part (S) determines the **upper bound of the speedup**.

#### 4.6.3 Numerical Table

| S (serial fraction) | N=2 | N=4 | N=8 | N=16 | N→∞ |
|:---:|:---:|:---:|:---:|:---:|:---:|
| **5%** | 1.90 | 3.48 | 5.93 | 9.14 | **20** |
| **10%** | 1.82 | 3.08 | 4.71 | 6.40 | **10** |
| **25%** | 1.60 | 2.28 | 2.91 | 3.37 | **4** |
| **50%** | 1.33 | 1.60 | 1.78 | 1.88 | **2** |

*Table 34. Amdahl's law speedup by serial fraction and number of cores*

#### 4.6.4 Concrete Calculation Example

**S = 25% (75% parallel)**:
- N=2: `1 / (0.25 + 0.75/2) = 1 / 0.625 ≈ 1.6x`
- N=4: `1 / (0.25 + 0.75/4) = 1 / 0.4375 ≈ 2.28x`
- N=8: `1 / (0.25 + 0.75/8) = 1 / 0.34375 ≈ 2.91x`
- N=∞: `1 / 0.25 = 4x` (maximum)

#### 4.6.5 Practical Implications

1. **Minimizing the serial part** matters more than adding cores
2. An **inherent limit** of parallel programming
3. With S = 50%, **at most 2x** is possible regardless of the number of cores

### 4.7 Two Types of Parallelism

#### 4.7.1 Data Parallelism

Applies **the same operation to distributed data**.

```text
Array: [1, 2, ..., 1000]
Thread 0: sum of 1-250       (same operation)
Thread 1: sum of 251-500     (same operation)
Thread 2: sum of 501-750     (same operation)
Thread 3: sum of 751-1000    (same operation)
```

*Figure 14. Splitting an array for summation under data parallelism*

- Suited for: arrays and matrices, image processing, scientific computing
- Scalability: linear in the data size

#### 4.7.2 Task Parallelism

Distributes **different operations across threads**.

```text
Array: [1, 2, ..., 1000]
Thread 0: compute the mean
Thread 1: compute the standard deviation
Thread 2: find the maximum
Thread 3: find the minimum
```

*Figure 15. Assigning a different operation to each thread under task parallelism*

- Suited for: web browsers, microservices
- Scalability: depends on the number of tasks

In practice, a **hybrid** is common.

### 4.8 User Threads and Kernel Threads

#### 4.8.1 Two Levels

**User thread**: Managed by a user-space library. Not recognized by the kernel.
- The library maintains its own thread table
- Disadvantage: one thread blocks → the whole process blocks

**Kernel thread**: Managed directly by the OS kernel. Recognized by the kernel's scheduler.
- Advantage: when one thread blocks, the other threads keep running
- Supported by default in modern OSes

### 4.9 Multithreading Models 🎯

#### 4.9.1 Many-to-One

Many user threads → one kernel thread.

- **Disadvantages (fatal)**:
  - One thread's blocking system call → **the whole process blocks**
  - **No parallelism** (only one kernel thread is scheduled at a time)
  - Uses only one core on a multicore system
- Old Solaris, early Java Green Threads: **almost never used**

#### 4.9.2 One-to-One: The Modern Standard

User thread ↔ kernel thread 1:1.

- **Advantages**:
  - Other threads can run even if one thread blocks
  - True parallelism
- **Disadvantage**: limited number of threads (kernel resources)
- Standard in **Linux, Windows**

#### 4.9.3 Many-to-Many

N user threads ↔ M kernel threads (M ≤ N).

- **Advantages**: flexibility, nonblocking, parallelism
- **Disadvantage**: complex implementation (upcall mechanism)
- Goroutines in Go and virtual threads in Java 21+ are its modern reappearance

#### 4.9.4 Two-Level

Many-to-many + 1:1 binding for important threads. Used in old Solaris and IRIX, and almost never used today.

#### 4.9.5 Final Comparison

| Model | Mapping | Advantages | Disadvantages | Today |
|---|---|---|---|---|
| Many-to-one | N:1 | Efficient management | **Blocking propagates, no parallelism** | ✗ |
| **One-to-one** | 1:1 | Parallel, stable | Limited number of threads | **Standard ✓** |
| Many-to-many | N:M | Flexible, parallel + nonblocking | Complex implementation | Limited |
| Two-level | N:M + 1:1 | Flexible + priority guarantee | Very complex | Almost never used |

*Table 35. Comparison of multithreading models*

🎯 **Why one-to-one became the standard**: As the number of processing cores grew, the many-to-many rationale of saving kernel threads weakened, and the **simplicity and stability** of one-to-one won.

### 4.10 Pthreads: The Standard API

POSIX Pthreads = the IEEE 1003.1c standard. It is only a **specification**, and the implementation is up to the OS designer.

#### 4.10.1 Core API

```c
int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);

int pthread_join(pthread_t thread, void **retval);
void pthread_exit(void *retval);
int pthread_attr_init(pthread_attr_t *attr);
```

#### 4.10.2 Analysis of pthread_create

Four arguments:
1. **`pthread_t *thread`**: output, the new thread ID
2. **`const pthread_attr_t *attr`**: attributes (NULL = defaults)
3. **`void *(*start_routine)(void*)`**: pointer to the thread function
4. **`void *arg`**: argument passed to the function (generic pointer)

**Function signature `void *(*)(void *)`**: Using a generic pointer allows **any type** to be passed.

#### 4.10.3 Practical Example: Integer Summation

```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sum;  // global: shared by all threads

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

Running `./a.out 5`:
```text
sum = 15
```

#### 4.10.4 A Common Bug: Passing Arguments ⚠️

```c
// ❌ Dangerous: every thread shares the same address &i
for (int i = 0; i < 4; i++) {
    pthread_create(&t[i], NULL, f, &i);
}
```

**Problem**: Because `pthread_create` is asynchronous, `i` may already have changed by the time the thread reads `*(int*)arg` → race condition.

```c
// ✅ Correct: separate storage for each thread
int ids[4];
for (int i = 0; i < 4; i++) {
    ids[i] = i;
    pthread_create(&t[i], NULL, f, &ids[i]);
}
```

### 4.11 Race Conditions and Synchronization 🎯

#### 4.11.1 Race Condition

```c
global_sum += array[i];
```

At the CPU level, this single line is a **three-step non-atomic operation**:
1. LOAD: read `global_sum` → register
2. ADD: add
3. STORE: write the result

**When two threads run at the same time**:
```text
Thread A              global_sum     Thread B
   read → 100           100            read → 100
   add  → 110                          add  → 105
   write → 110          110            write → 105
                        ↑
                     final 105 (A's +10 is lost!)
```

*Figure 16. Lost update of global_sum when two threads update it concurrently*

#### 4.11.2 Solutions

**Method 1. Separate storage (the simplest)**:
```c
int partial_sum[NUM_THREADS];

void *work(void *arg) {
    int id = *(int *)arg;
    partial_sum[id] = 0;
    for (int i = start; i < end; i++)
        partial_sum[id] += array[i];  // no conflict!
    return NULL;
}
```

**Method 2. Mutex (detailed in Week 9)**:
```c
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_lock(&mutex);
global_sum += value;
pthread_mutex_unlock(&mutex);
```

### 4.12 Java Threads

#### 4.12.1 Two Ways to Create a Thread

```java
// Method 1: extend Thread (not recommended)
class Task extends Thread {
    public void run() { /* ... */ }
}
new Task().start();

// Method 2: implement Runnable (recommended)
class Task implements Runnable {
    public void run() { /* ... */ }
}
new Thread(new Task()).start();

// Lambda (Java 8+)
new Thread(() -> { /* ... */ }).start();
```

**Why Runnable is recommended**: Java does not allow multiple inheritance → compatible with extending another class.

#### 4.12.2 start() vs run() ⚠️

```java
task.run();   // ❌ runs in the same thread (no new thread is created!)
task.start(); // ✅ creates a new thread and then runs run()
```

#### 4.12.3 join()

```java
Thread worker = new Thread(new Task());
worker.start();
try {
    worker.join();  // wait for completion
} catch (InterruptedException ie) { }
```

---

### Chapter 4 Wrap-Up 🎯

**Big Picture of Chapter 4**: A thread is a lightweight unit of execution within a process that shares resources. Multithreading gives four benefits (responsiveness, resource sharing, economy, scalability) but leads to race conditions. The one-to-one model is the modern standard. Amdahl's law is the absolute limit of parallelization.

---

## Chapter 5. Implicit Threading and Threading Issues (Week 5)

### 5.1 Implicit Threading: Why It Is Needed

Explicit threading (`pthread_create`) is impractical because hundreds of threads must be managed directly. Solution: **define only the tasks** and **delegate scheduling to a library**.

**Five representative techniques (T-F-O-G-I)**:
- **T**hread Pool
- **F**ork-Join
- **O**penMP
- **G**CD (Grand Central Dispatch)
- **I**ntel TBB

### 5.2 Thread Pool

#### 5.2.1 Concept

Creates a **fixed number of worker threads** in advance. When a task is **submitted to the queue**, a worker takes it out and runs it. After finishing, the worker returns to the pool and is reused.

#### 5.2.2 Three Benefits 🎯

1. **Speed**: reuse eliminates creation cost
2. **Resource limits**: caps the number of concurrent threads (protects the system)
3. **Separation**: separates task definition from the execution strategy (flexible, e.g., delayed or periodic execution)

#### 5.2.3 Java ExecutorService

```java
ExecutorService pool = Executors.newFixedThreadPool(4);
for (int i = 0; i < 100; i++)
    pool.execute(new Task(i));
pool.shutdown();
```

**Factory methods**:
| Method | Use |
|---|---|
| `newSingleThreadExecutor()` | Size 1, sequential |
| `newFixedThreadPool(n)` | Fixed n (**standard for servers**) |
| `newCachedThreadPool()` | Adjusts dynamically |

*Table 36. ExecutorService factory methods*

#### 5.2.4 Callable/Future: Returning Results

`Runnable` has no return value (void). When a result is needed, use `Callable<V>`.

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

// usage
Future<Integer> result = pool.submit(new Summation(10));
int answer = result.get();  // blocks until the result is ready
pool.shutdown();
```

### 5.3 Fork-Join

#### 5.3.1 Pattern

Ideal for recursive **divide and conquer**.

```text
fork ─┬→ Task 1
      ├→ Task 2
      └→ Task 3
         ↓
       join
         ↓
      combine
```

*Figure 17. Fork and join in the Fork-Join pattern*

#### 5.3.2 Java Example: Array Summation

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

#### 5.3.3 Work Stealing

Each thread maintains its own **deque**. When its queue is empty, it steals a task from **the opposite end** of another thread's deque.

- **Decentralized load balancing**: coordinates itself without a central scheduler
- **Cache-friendly**: the owner works LIFO (locality), the thief takes FIFO
- **Minimal contention**: the two ends of the deque are used separately

### 5.4 OpenMP

#### 5.4.1 Basic Use

```c
#include <omp.h>

#pragma omp parallel
{
    int tid = omp_get_thread_num();
    printf("Thread %d\n", tid);
}
```

Compiling: `gcc -fopenmp prog.c`

#### 5.4.2 parallel for

```c
#pragma omp parallel for
for (int i = 0; i < N; i++) {
    c[i] = a[i] + b[i];
}
```

- The iteration space is **divided by the number of threads**
- **Implicit barrier**: execution continues only after every thread finishes

#### 5.4.3 Reduction

```c
int sum = 0;
#pragma omp parallel for reduction(+:sum)
for (int i = 0; i < N; i++) sum += arr[i];
```

**Internally**: Each thread accumulates into a **local copy** → the copies are combined safely at the end.

**Supported operators**: `+`, `-`, `*`, `/`, `&`, `|`, `^`, `&&`, `||`, `max`, `min`

#### 5.4.4 Data-Sharing Clauses

- `shared(var)`: shared by all threads (default)
- `private(var)`: a copy for each thread
- `reduction(op:var)`: safe combination

#### 5.4.5 Main Functions

| Function | Use |
|---|---|
| `omp_get_thread_num()` | Current thread ID |
| `omp_get_num_threads()` | Total number of threads |
| `omp_set_num_threads(n)` | Sets the number of threads |
| `omp_get_wtime()` | Wall-clock time (benchmarking) |

*Table 37. Main OpenMP functions*

Environment variable: `OMP_NUM_THREADS=4 ./prog`

### 5.5 GCD (Grand Central Dispatch)

Developed by Apple for macOS/iOS. Blocks/closures are submitted to **dispatch queues**.

#### 5.5.1 Queue Types

- **Serial queue**: FIFO, one at a time
- **Concurrent queue**: FIFO dequeue, several tasks run at once. Priority through QoS classes.

#### 5.5.2 QoS Classes

| Class | Use | Priority |
|---|---|---|
| `USER_INTERACTIVE` | UI, immediate response | Highest |
| `USER_INITIATED` | The user is waiting | High |
| `UTILITY` | Long-running background | Medium |
| `BACKGROUND` | Indexing, backup | Low |

*Table 38. GCD QoS classes*

#### 5.5.3 Usage Example

```c
dispatch_queue_t queue = dispatch_get_global_queue(
    DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
dispatch_async(queue, ^{ printf("Async\n"); });
```

### 5.6 Intel TBB

A C++ template library. No special compiler or language extension is required.

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

### 5.7 Comparison of the Five Techniques

| Technique | Language | Core idea | Suited for |
|---|---|---|---|
| Thread Pool | Java, C# | Reuse | General-purpose servers |
| Fork-Join | Java | Divide and conquer | Recursive parallelism |
| OpenMP | C/C++/Fortran | Directives | HPC |
| GCD | macOS/iOS | Queues + QoS | Mobile UI |
| Intel TBB | C++ | Lambdas + cache awareness | High-performance C++ |

*Table 39. Comparison of five implicit threading techniques*

### 5.8 Threading Issue 1: fork() / exec() Semantics

#### 5.8.1 Problem

When one thread of a multithreaded process calls fork():
- **Option A**: duplicate all threads
- **Option B**: duplicate only the calling thread

**POSIX default**: duplicate only the calling thread.

#### 5.8.2 The Danger of an Orphaned Lock ⚠️

```c
// parent process
Thread 2: pthread_mutex_lock(&mutex);  // acquires the mutex
          // ...working...

Thread 3: fork();  // only Thread 3 is duplicated into the child

// child process
Child: pthread_mutex_lock(&mutex);
      → DEADLOCK! (the mutex is locked, and no thread will release it)
```

#### 5.8.3 Solution: fork + exec

```c
pid_t pid = fork();
if (pid == 0) {
    execlp("echo", "echo", "Hello", NULL);  // exec immediately
    perror("exec"); exit(1);
}
```

Because `exec()` replaces the entire process image, the problem of the inherited mutex goes away automatically.

### 5.9 Threading Issue 2: Signal Handling

#### 5.9.1 Synchronous vs Asynchronous Signals

- **Synchronous (SIGSEGV, SIGFPE)**: caused by the process's own action → delivered to the thread that caused it
- **Asynchronous (SIGTERM, SIGALRM)**: external source → **the problem of which thread should receive it**

#### 5.9.2 Four Delivery Options

1. To the thread that caused it (synchronous signals)
2. To every thread (broadcast)
3. To threads that have not blocked it (POSIX default)
4. **To a specific dedicated thread** (recommended)

#### 5.9.3 Recommended Pattern: A Dedicated Thread

```c
void *signal_handler_thread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int sig;
    while (1) {
        sigwait(set, &sig);  // blocking
        switch (sig) {
            case SIGTERM: exit(0);
            case SIGINT: /* handle */ break;
        }
    }
}

int main() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);

    pthread_sigmask(SIG_BLOCK, &set, NULL);  // block in all threads
    pthread_t handler;
    pthread_create(&handler, NULL, signal_handler_thread, &set);
    // ...
}
```

### 5.10 Threading Issue 3: Thread Cancellation

#### 5.10.1 Two Modes

| | Asynchronous cancellation | Deferred cancellation (default) |
|---|---|---|
| When | Immediately | Only at cancellation points |
| Safety | **Dangerous** (resource leaks) | Safe |

*Table 40. Comparison of asynchronous and deferred cancellation*

#### 5.10.2 Cancellation Points

Cancellation points in POSIX:
- Blocking system calls: `read`, `write`, `open`, `sleep`, `pause`
- Synchronization: `pthread_join`, `pthread_mutex_lock`, `pthread_cond_wait`
- Explicit: `pthread_testcancel()`

#### 5.10.3 Cleanup Handlers

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

    pthread_cleanup_pop(1);  // 1 = run the cleanup
    return NULL;
}
```

#### 5.10.4 Java Uses Interrupts

Java does not provide asynchronous cancellation and uses **interrupts** instead.
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

### 5.11 Threading Issue 4: TLS (Thread-Local Storage)

#### 5.11.1 Why It Is Needed

**Problem**: Take `errno`: if Thread A sets `errno=ENOENT` because `open()` failed, and then Thread B succeeds and overwrites it with 0, A sees the wrong error information.

**Solution**: An independent copy for each thread = TLS.

#### 5.11.2 Difference from Local Variables

| | Local variable | TLS |
|---|---|---|
| Declaration | Inside a function | Global storage |
| Lifetime | Only during the function call | **The entire lifetime of the thread** |
| Visibility | Function scope | **All functions** |
| Storage | Stack | A special per-thread area |

*Table 41. Comparison of local variables and TLS*

#### 5.11.3 Implementations by Language

```c
// C (GCC)
__thread int threadID;

// C11 standard
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

#### 5.11.4 Lab Example

```c
int shared_var = 0;        // race condition!
__thread int tls_var = 0;  // safe

void *worker(void *arg) {
    for (int i = 0; i < 100000; i++) {
        shared_var++;   // race: inaccurate result
        tls_var++;      // safe: independent per thread
    }
    // tls_var is always 100000 (correct)
    // shared_var is indeterminate
}
```

### 5.12 Threading Issue 5: LWP and Scheduler Activation

#### 5.12.1 LWP (Lightweight Process)

An **intermediate layer** between user threads and kernel threads. It appears to the library as a **virtual CPU**.

```text
User Thread
     ↓
    LWP (virtual processor)
     ↓
 Kernel Thread
     ↓
 Physical CPU
```

*Figure 18. Layers of user threads, LWPs, kernel threads, and physical CPUs*

#### 5.12.2 Upcall

**Two-way communication** between the kernel and the library in the many-to-many model.

```text
Scenario: Thread A blocks in read()
1. Kernel: "Thread A blocking" → an upcall occurs
2. Library upcall handler: assigns another thread to the LWP
3. I/O completes → the kernel upcalls again: "A is ready"
```

🎯 **Complexity**: Because implementation is very difficult, most modern OSes have returned to **one-to-one**.

### 5.13 Windows Thread Structures

Windows uses the **one-to-one model**.

| Structure | Location | Contents |
|---|---|---|
| **ETHREAD** | Kernel | Start address, parent, pointer to KTHREAD |
| **KTHREAD** | Kernel | State, priority, kernel stack |
| **TEB** | User | TID, user stack, TLS array |

*Table 42. Windows thread data structures*

### 5.14 Linux's Unified Model: clone()

#### 5.14.1 An Innovative Design

Linux **does not distinguish between processes and threads**. Both are **tasks** that use the common `task_struct`. The level of sharing is controlled by `clone()` flags.

#### 5.14.2 clone Flags

| Flag | Shared |
|---|---|
| `CLONE_VM` | Virtual memory |
| `CLONE_FS` | File system information |
| `CLONE_FILES` | fd table |
| `CLONE_SIGHAND` | Signal handlers |
| `CLONE_THREAD` | Thread group |
| `CLONE_NEWNS/PID/NET` | Namespaces (containers) |

*Table 43. clone() flags and what they share*

#### 5.14.3 Three Cases

- **fork()** = `clone(0)`: shares nothing
- **pthread_create** ≈ `clone(CLONE_VM|FS|FILES|SIGHAND|THREAD)`: shares almost everything
- **Container** = `clone(..|CLONE_NEWNS|NEWPID|NEWNET)`: partial isolation

#### 5.14.4 Implementing Containers

Docker and LXC use this model:
```c
clone(container_init, stack,
    CLONE_VM
    | CLONE_NEWNS     // file system isolation
    | CLONE_NEWPID    // process tree isolation
    | CLONE_NEWNET,   // network isolation
    NULL);
```

🎯 **Result**: Containers, which are like lightweight virtual machines, are implemented naturally.

---

### Chapter 5 Wrap-Up 🎯

**Big Picture of Chapter 5**: Developers define only the tasks, and the library manages the threads (implicit threading). But threads carry five tricky issues (fork/exec, signals, cancellation, TLS, LWP). Linux's clone() unifies processes and threads and naturally supports even containers.

---

## Chapter 6. Basic CPU Scheduling Algorithms (Week 6)

### 6.1 The Premise of Scheduling: The CPU-I/O Burst Cycle

#### 6.1.1 Process Execution Pattern

Process execution is **an alternation of CPU bursts ↔ I/O bursts**.

```text
CPU burst → I/O burst → CPU burst → I/O burst → ... → exit
 (compute)  (disk, network)  (compute)  (I/O)
```

*Figure 19. Alternation of CPU bursts and I/O bursts*

#### 6.1.2 Two Types of Processes

| Type | Characteristic | Examples |
|---|---|---|
| **I/O-bound** | Many short CPU bursts | Browser, DB |
| **CPU-bound** | Few long CPU bursts | Scientific computing, rendering |

*Table 44. Burst characteristics of I/O-bound and CPU-bound processes*

With a mix, both the CPU and the devices are used efficiently.

### 6.2 Scheduling Decision Points and Preemption

#### 6.2.1 Four Decision Points

1. Running → Waiting (voluntary, nonpreemptive)
2. **Running → Ready (preemptive)**
3. **Waiting → Ready (preemption possible)**
4. Termination

#### 6.2.2 Preemptive vs Nonpreemptive

- **Nonpreemptive**: schedules only at 1 and 4, and the process gives up the CPU voluntarily. Simple.
- **Preemptive**: also decides at 2 and 3, and the OS forcibly takes back the CPU. Good responsiveness, but shared data must be protected. **The standard in modern OSes**.

### 6.3 Dispatcher

**Definition**: The module that **actually transfers control** to the process chosen by the scheduler.

**Three roles**:
1. **Context switching**: saving/restoring registers
2. **Switching between user and kernel mode**
3. **Jumping to the PC**: to the proper location in the user program

**Dispatch latency**: The time from stopping one process to starting another. Typically 1-10 µs.

🔗 **Connection**: This latency sets the lower bound for choosing the round-robin (RR) quantum (see Section 6.8.4).

### 6.4 Scheduling Criteria 🎯

Five evaluation metrics:

| Criterion | Meaning | Direction |
|---|---|---|
| **CPU utilization** | Fraction of time the CPU is not idle | Maximize |
| **Throughput** | Processes completed per unit time | Maximize |
| **Turnaround time** | From submission to completion | Minimize |
| **Waiting time** | **Time spent in the ready queue** | **Minimize (key)** |
| **Response time** | From submission to the first response | Minimize (interactive) |

*Table 45. Five scheduling criteria*

**Composition of turnaround time**:
```text
Turnaround time = waiting time + CPU burst time + I/O time
```

🎯 **Why waiting time is the key metric**: CPU bursts and I/O time are properties of the process itself, so the scheduler cannot change them. What scheduling affects is **only waiting time**.

### 6.5 FCFS (First-Come, First-Served)

#### 6.5.1 Rule

Run in order of arrival. Nonpreemptive.

#### 6.5.2 Example Calculation 🧪

**Processes**: P1(24), P2(3), P3(3), assumed to arrive in this order

**Gantt chart (order P1-P2-P3)**:
```text
| P1         | P2 | P3 |
0           24   27   30
```

*Figure 20. FCFS Gantt chart (order P1-P2-P3)*

**Waiting time**:
- P1: 0
- P2: 24
- P3: 27
- **Average: (0+24+27)/3 = 17**

**What if the order is P2-P3-P1?**:
```text
| P2 | P3 | P1         |
0    3    6           30
```

*Figure 21. FCFS Gantt chart (order P2-P3-P1)*

- Average waiting: (0+3+6)/3 = **3**

#### 6.5.3 Convoy Effect 🎯

**Problem**: If a long process is at the front, all the short processes behind it wait → the average waiting time explodes.

FCFS is **extremely sensitive to arrival order**: even for the same set of processes, the order can make an extreme difference such as 17 vs 3.

#### 6.5.4 Summary of Characteristics

- **Advantages**: simple, easy to implement
- **Disadvantages**: convoy effect, high performance variability
- **Response time**: poor

### 6.6 SJF (Shortest Job First)

#### 6.6.1 Rule

The process whose next CPU burst is **the shortest** goes first. Nonpreemptive.

#### 6.6.2 Why It Is Optimal

It is proven that it **minimizes the average waiting time in theory**.

💡 **Intuition**: Handling short jobs first minimizes the **total time** that the processes behind them wait. A long job takes a long time anyway, so pushing it back increases only its own waiting time while reducing the waiting times of many short jobs.

#### 6.6.3 Example Calculation 🧪

**Processes and bursts**: P1(6), P2(8), P3(7), P4(3)

**SJF Gantt chart (all assumed to arrive at t=0)**:
```text
| P4 | P1    | P3     | P2      |
0    3       9       16        24
```

*Figure 22. SJF Gantt chart*

**Waiting time**:
- P4: 0, P1: 3, P3: 9, P2: 16
- **Average: (0+3+9+16)/4 = 7**

**The same processes under FCFS (order P1-P2-P3-P4)**:
```text
| P1    | P2      | P3     | P4 |
0        6       14      21   24
```

*Figure 23. FCFS Gantt chart for the same processes (order P1-P2-P3-P4)*

- Average: (0+6+14+21)/4 = **10.25**

#### 6.6.4 The Practical Barrier: Prediction Is Needed ⚠️

**Problem**: In reality, the length of the next CPU burst **cannot be known**.

**Solution**: Predict it from the past with **exponential averaging**.

$$ \tau_{n+1} = \alpha \cdot t_n + (1-\alpha) \cdot \tau_n $$

- $t_n$: length of the nth actual burst
- $\tau_{n+1}$: the (n+1)th predicted value
- $\alpha$: weight of the most recent value (0 ≤ α ≤ 1)

**Interpreting α**:
- α = 0: uses only the past prediction (does not reflect changes)
- α = 1: uses only the most recent value (ignores the past)
- α = 0.5 (common): balanced

#### 6.6.5 The Starvation Problem

**Starvation**: A long process keeps getting pushed back by short ones and never runs.

**Solution**: **Aging**, that is, raising the priority as the waiting time grows.

### 6.7 SRTF (Shortest Remaining Time First): Preemptive SJF

#### 6.7.1 Rule

If the burst of a newly arrived process is shorter than the **remaining time** of the running process, **preemption** occurs.

#### 6.7.2 Example Calculation 🧪

**Processes** (arrival time, burst):
- P1 (0, 8), P2 (1, 4), P3 (2, 9), P4 (3, 5)

**SRTF Gantt**:
```text
t=0: P1 arrives, starts running (8 left)
t=1: P2 arrives (4 < P1's remaining 7). Preempt! Run P2
t=2: P3 arrives (9). P2's remaining 3 < 9. Continue P2
t=3: P4 arrives (5). P2's remaining 2 < 5. Continue P2
t=5: P2 done. Candidates: P1 (7 left), P3(9), P4(5). Choose P4
t=10: P4 done. Candidates: P1(7), P3(9). P1
t=17: P1 done. Run P3
t=26: P3 done
```

```text
| P1 | P2     | P4     | P1      | P3       |
0    1        5       10        17        26
```

*Figure 24. SRTF Gantt chart*

**Calculating waiting time**:
- P1: starts at t=0, is preempted at t=1, resumes at t=10, and completes at t=17
  - Waiting = 10-1 = 9 (runs t=0-1, waits t=1-10, runs t=10-17)
- P2: 0 (arrives at t=1, starts at t=1)
- P3: 17-2 = 15
- P4: 5-3 = 2
- **Average: (9+0+15+2)/4 = 6.5**

#### 6.7.3 Characteristics

- Generally **lower average waiting time** than SJF
- **Higher risk of starvation** (long jobs are pushed back more often)

### 6.8 Round Robin (RR)

#### 6.8.1 Rule

Each process is given a **time quantum q**. When the quantum expires, the process is **preempted** and moved to the back of the queue.

#### 6.8.2 Key Properties

- **No starvation**: with N processes, the maximum wait of a process = (N-1) × q
- **Excellent response time**: best for interactive use

#### 6.8.3 Example Calculation 🧪

**Processes**: P1(24), P2(3), P3(3), all arriving at t=0, q = 4

**Gantt chart**:
```text
| P1 | P2 | P3 | P1 | P1 | P1 | P1 | P1 |
0    4    7   10   14   18   22   26   30
```

*Figure 25. RR Gantt chart (q = 4)*

- P1 runs 6 times for 4 each (24 = 4×6): 4, 4, 4, 4, 4, 4 = 24 in total
- Actual order: P1(0-4) → P2(4-7) → P3(7-10) → P1(10-14) → P1(14-18) → P1(18-22) → P1(22-26) → P1(26-30)

**Waiting time**:
- P1: completion time 30, waiting = 30 - 24 = 6
- P2: completion 7, waiting = 7 - 3 = 4
- P3: completion 10, waiting = 10 - 3 = 7
- **Average: (6+4+7)/3 = 5.67**

#### 6.8.4 Choosing the Time Quantum 🎯

**Dilemma**:
- **Too large** → converges to FCFS (worse response time)
- **Too small** → context switching overhead dominates

**80% rule**: Set q so that **80% of CPU bursts complete within one quantum**.

**Numerical example**:
- With a dispatch latency of 10 µs and q = 100 µs, the overhead is about 10%
- With q = 1 ms, the overhead is 1%
- With q = 10 ms, the overhead is 0.1%, but the response time can grow to 10ms

#### 6.8.5 Characteristics

- **Advantages**: fair, no starvation, good response time
- **Disadvantages**: the average waiting time can be worse than SJF, and the choice of quantum matters

### 6.9 Priority Scheduling

#### 6.9.1 Rule

Each process has a **priority number**. The highest priority goes first.

- Usually **a smaller number = a higher priority**
- Both preemptive and nonpreemptive versions are possible
- **SJF is the special case where priority = 1/(next burst)**

#### 6.9.2 Starvation → Solved by Aging

**Aging**: The longer the waiting time, the higher the priority.
Example: raising the priority by 1 every minute → eventually every process gets a chance to run.

### 6.10 Overall Comparison of the Four Algorithms 🎯

| | Type | Average waiting | Response | Starvation | Advantages | Disadvantages |
|---|---|---|---|---|---|---|
| **FCFS** | Nonpreemptive | Large | Poor | X | Simple | **Convoy effect** |
| **SJF** | Nonpreemptive | **Minimal** | Medium | O | Optimal | Prediction needed |
| **SRTF** | Preemptive | Lower than SJF | Good | O | Even better | Overhead |
| **RR** | Preemptive | Large | **Very good** | X | **Fair, responsive** | Choosing the quantum |
| **Priority** | Either | Varies | Varies | O→aging | Reflects importance | Risk of starvation |

*Table 46. Overall comparison of basic scheduling algorithms*

### 6.11 xv6 Context Switching: struct context and swtch.S

#### 6.11.1 struct context

```c
struct context {
    uint64 ra;     // return address (PC)
    uint64 sp;     // stack pointer
    uint64 s0;     // callee-saved registers
    uint64 s1;
    // ...
    uint64 s11;
};
```

**Why only callee-saved registers?**: The caller-saved registers (t0-t6, a0-a7) are already saved on the stack by the C compiler, because the C code that calls swtch() assumes the call returns.

#### 6.11.2 swtch.S Flow

```assembly
swtch:
    # save the current registers into old.context (*a0)
    sd ra, 0(a0)
    sd sp, 8(a0)
    sd s0, 16(a0)
    # ... s1-s11 ...

    # restore registers from new.context (*a1)
    ld ra, 0(a1)
    ld sp, 8(a1)
    ld s0, 16(a1)
    # ... s1-s11 ...

    ret  # jumps to the new process's PC held in ra
```

🎯 **Key point**: When `ret` executes, it **jumps to another process's PC**. The code that called swtch() returns **in the context of another process**.

---

### Chapter 6 Wrap-Up 🎯

**Big Picture of Chapter 6**: Among the five criteria of scheduling (CPU utilization, throughput, turnaround, waiting, and response time), waiting time is the key. FCFS (simple but suffers the convoy effect), SJF (optimal but needs prediction), SRTF (preemptive SJF), RR (fair and responsive). Every algorithm is a compromise among trade-offs.

---

## Chapter 7. Advanced Scheduling: Multiprocessor and Real-Time (Week 7)

### 7.1 Multilevel Queue (MLQ)

#### 7.1.1 Concept

Processes are placed in **separate queues** by type. Each queue has its **own scheduling** policy.

```text
| Foreground Queue | → RR (short quantum)
| Background Queue | → FCFS

Between queues: fixed priority (Foreground first)
```

*Figure 26. Organization of a multilevel queue*

#### 7.1.2 Limitation

**Static classification**: A process is assigned to one queue **permanently** → changes in its behavior are not reflected.

### 7.2 Multilevel Feedback Queue (MLFQ) 🎯

#### 7.2.1 Three Rules

1. **New process**: enters the top queue (Q0)
2. **Demotion**: uses up the queue's quantum → moves to the next lower queue
3. **Promotion (aging)**: waits a long time → moves to a higher queue

#### 7.2.2 Example Configuration

| Queue | Scheduling | Quantum | Role |
|---|---|---|---|
| Q0 | RR | 8 ms | Short/interactive |
| Q1 | RR | 16 ms | Medium jobs |
| Q2 | FCFS | Unlimited | Long CPU-bound |

*Table 47. Example MLFQ configuration*

#### 7.2.3 Why It Is Brilliant 🎯

There is **no need to know in advance what type a process is**. It is **classified automatically** by observing its behavior:
- Short jobs → finish in Q0 (low turnaround time)
- I/O-bound → gives up the CPU voluntarily on I/O → stays in Q0 (favors interactivity)
- CPU-bound → keeps using up its quantum → demoted to Q2
- A Q2 process that has waited long → promoted (prevents starvation)

🎯 **Result**: **Approximates SJF** while also **preventing starvation**. The default strategy of modern UNIX-like systems.

#### 7.2.4 Modern OS Examples

- **Linux O(1) scheduler** (before 2.6): 140-level MLFQ
- **Solaris Time-Sharing Class**
- **Windows**: multilevel queue + dynamic boost

### 7.3 Thread Scheduling: PCS vs SCS

#### 7.3.1 Two Contention Scopes

| | PCS (Process-Contention Scope) | SCS (System-Contention Scope) |
|---|---|---|
| Competition | Among threads within a process | **Among all threads in the system** |
| Model used | Many-to-one, many-to-many | **One-to-one** |
| Today | Limited | **Modern standard** |

*Table 48. Comparison of PCS and SCS*

#### 7.3.2 Pthreads scheduling contention scope

```c
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);  // SCS
// or PTHREAD_SCOPE_PROCESS
```

Linux and macOS **support only SCS** (because they use the one-to-one model).

### 7.4 Multiprocessor Scheduling

#### 7.4.1 SMP (Symmetric Multiprocessing)

Each CPU makes scheduling decisions **independently**.
- **Symmetric**: every CPU has the same role
- **A common ready queue** or **per-CPU queues** are possible

**Asymmetric**: Only the master CPU makes decisions, and the others run only user code. Simple but a bottleneck.

#### 7.4.2 Processor Affinity

**Concept**: The tendency to **keep a process on the same CPU**.

**Why it is needed**:
- When a process runs on CPU A, its data is loaded into A's cache
- If it moves to CPU B, B's cache is empty → **cache misses surge → lower performance**

| | Description |
|---|---|
| **Soft Affinity** | The same CPU when possible (most OSes) |
| **Hard Affinity** | Forcibly pinned to specific CPUs (`sched_setaffinity()`) |

*Table 49. Soft affinity and hard affinity*

#### 7.4.3 NUMA (Non-Uniform Memory Access)

**Problem**: In multisocket systems, **memory latency differs** from core to core.
- CPU 0 → memory A: 100 ns (local)
- CPU 0 → memory B: 200 ns (remote)

**Scheduling decision**: Place a process on **a CPU near its memory** (NUMA-aware).

#### 7.4.4 Load Balancing

- **Push migration**: an overloaded CPU → pushes work to other CPUs (periodic checks)
- **Pull migration**: an idle CPU → pulls work from a busy CPU's queue

⚠️ **Affinity vs load balancing conflict**: keeping the cache ← affinity / even distribution ← load balancing. A good scheduler strikes a balance.

#### 7.4.5 SMT (Simultaneous Multithreading) / Hyperthreading

**Several hardware threads** on one physical core. Another thread runs during a memory stall → higher efficiency.

**Scheduling hint**: Assign **other physical cores first** rather than the second HW thread of the same physical core → better parallelism.

### 7.5 Real-Time Scheduling

#### 7.5.1 Soft vs Hard

| | Soft Real-Time | Hard Real-Time |
|---|---|---|
| Deadline guarantee | Best effort | **Mandatory** |
| Effect of a violation | Degraded performance | **System failure** |
| Examples | Video streaming, games | Flight control, medical devices |

*Table 50. Comparison of soft real-time and hard real-time*

#### 7.5.2 Latency Components

- **Interrupt latency**: interrupt arrival → ISR start
- **Dispatch latency**: real-time task ready → actually running
- **Event latency**: event → service completed (the total)

#### 7.5.3 Rate-Monotonic (RMS)

**Static priority = 1 / period**. The shorter the period, the higher the priority.

🧪 **Example**:
- T1: period 50ms, execution 20ms
- T2: period 100ms, execution 35ms

RMS: T1 has priority (shorter period). T1 runs for 20ms every 50ms, and T2 completes its 35ms in the gaps.

**Utilization bound**:
$$ U \le N(2^{1/N} - 1) $$

| N | Bound |
|---|---|
| 1 | 1.00 |
| 2 | 0.828 |
| 3 | 0.779 |
| ∞ | **0.693 (about 69.3%)** |

*Table 51. RMS utilization bound by number of tasks*

Utilization above this bound **may miss deadlines**.

#### 7.5.4 EDF (Earliest-Deadline-First)

**Dynamic priority = the nearest deadline**. Priorities change at every point in time.

**Utilization**: **100%** is possible in theory.

**Condition**: The deadline of each task must be known.

#### 7.5.5 Priority Inversion & Inheritance

**Scenario**:
- T3 (low priority) locks a mutex
- T1 (high priority) waits for the mutex
- T2 (medium priority) preempts T3 and runs → T3 cannot release the mutex → **T1 is blocked indirectly by T2**

**Solution: the priority inheritance protocol**
- **Temporarily raise** the priority of the blocking T3 to T1's level
- After T3 releases the mutex, it returns to its original priority

🧪 **Famous case**: The 1997 reboot bug of the Mars Pathfinder lander. It was caused by this issue and fixed by enabling the inheritance protocol.

### 7.6 Operating System Examples

#### 7.6.1 Linux CFS (Completely Fair Scheduler)

**Key concepts**:
- **vruntime (virtual run time)**: the virtual run time of each task, weighted by priority (nice value)
- **RB-tree**: sorted by vruntime → selects the minimum in O(log N)

**What complete fairness means**:
- In the long run, every task receives the CPU **in proportion to its weight**
- The lower the nice value (the higher the priority), the more slowly vruntime increases

#### 7.6.2 Windows

**32 priority levels** (0-31). Two classes:
- Variable class (1-15)
- Real-time class (16-31)

**Dynamic priority boost**:
- Keyboard/mouse input → the process temporarily gets a boost
- Focused window → boost
- Result: **maximized interactive responsiveness**

#### 7.6.3 Solaris

Six priority classes:
- Time-sharing (TS)
- Interactive (IA)
- Fair share (FSS)
- Fixed priority (FP)
- System (SYS)
- Real-time (RT)

Within each class, MLFQ is used.

### 7.7 Scheduling Evaluation Methods 🎯

#### 7.7.1 Four Methods

| Method | Accuracy | Cost | Limitation |
|---|---|---|---|
| **Deterministic modeling** | Precise | Low | Only a specific workload |
| **Queueing model** | General | Medium | Hard to compute for realistic policies |
| **Simulation** | High | High | Implementation cost |
| **Implementation** | **Perfect** | Very high | **User behavior changes** |

*Table 52. Comparison of scheduling evaluation methods*

#### 7.7.2 Little's Formula

$$ n = \lambda \cdot W $$

- **n**: average queue length
- **λ**: arrival rate (processes/second)
- **W**: average waiting time

🧪 **Example**: λ=7/sec, n=14 → W = 14/7 = **2 seconds**

**Generality**: Holds for any arrival distribution and any service time distribution. The only assumption is a **steady state**.

**Limitation**: Computing n and λ is complicated for realistic policies.

#### 7.7.3 Simulation

**Method**: A software model of the scheduler + synthetic/trace workloads.

**Workload sources**:
- **Synthetic**: random number generation (convenient, may differ from reality)
- **Trace**: real recordings (accurate, large in size)

#### 7.7.4 Implementation: The Only Fully Accurate Method

Implement the scheduler in a real OS and deploy it to users.

⚠️ **Biggest limitation**: **User behavior changes**.

**Historical example**: Early multiuser systems automatically boosted I/O-bound processes → excellent performance in simulation. After deployment, users noticed and wrapped CPU-bound work in fake I/O (e.g., writing to `/dev/null`) → they fooled the scheduler → the evaluation collapsed.

🎯 **Lesson**: The real environment is **an irreplaceable evaluation method**.

---

### Chapter 7 Wrap-Up 🎯

**Big Picture of Chapter 7**: MLFQ is a brilliant design that approximates SJF while preventing starvation. On multiprocessors, affinity (cache) and load balancing (evenness) conflict. In real-time systems the deadline guarantee is the key: RMS (static, 69.3% bound) vs EDF (dynamic, 100%). Priority inversion can be fatal, as on the Mars Pathfinder, but it is solved by the inheritance protocol. The best evaluation method is implementation, but it can collapse because of changes in user behavior.

---

## Chapter 8. The Big Picture: How the Concepts of Seven Weeks Fit Together

### 8.1 Why the Topics Are Learned in This Order

The **designed learning curve** from Week 1 to Week 7:

```text
W01: OS definition, dual mode (protection)
         ↓ why is protection needed? to run several programs
W02: Processes (the abstraction of execution)
         ↓ processes are isolated, yet they need to cooperate
W03: IPC (how to cooperate)
         ↓ IPC is heavy. A lightweight unit sharing one address space?
W04: Threads (lightweight concurrent execution)
         ↓ explicit threading is complex. A better abstraction?
W05: Implicit threading, threading issues
         ↓ in the end there is one CPU (or a few). How to divide it?
W06: Basic scheduling algorithms
         ↓ reality: multicore, real-time, diverse requirements
W07: Advanced scheduling
```

*Figure 27. How the topics flow from Week 1 to Week 7*

### 8.2 Seven Key Connections

#### 🔗 Connection 1: Dual Mode → System Calls → Every OS Function

The OS created dual mode to protect itself. As a result, **system calls** are the only passage for user programs. `fork`, `exec`, `pipe`, `shm_open`, and `pthread_create` are all among those system calls. **Every OS function is built on top of this single protection mechanism**.

#### 🔗 Connection 2: Process Isolation ↔ The Need for IPC

Processes must be isolated for safety (Week 2). Yet they need to cooperate (Week 3). Message passing is safe but slow, and shared memory is fast but synchronization is risky. **This tension leads to synchronization in Week 9**.

#### 🔗 Connection 3: Threads Are a Double-Edged Sword

The advantage of threads (shared memory) is also their disadvantage (race conditions). Week 4 avoided it with `partial_sum[id]`, and Week 5 partly solved it with `reduction` and `ThreadLocal`, but **the fundamental solution comes in Week 9** (semaphores, mutexes, condition variables).

#### 🔗 Connection 4: Amdahl's Law → The Justification for Scheduling

Parallelization has an inherent limit (S). Because there is a limit no matter how many cores are added, **scheduling that divides a limited CPU efficiently** (Weeks 6-7) still matters. "With multicore, there is no need to worry about scheduling" is a wrong intuition.

#### 🔗 Connection 5: PCB → Context Switching → Scheduling Overhead

Scheduling is impossible without the PCB (Week 2). Saving/restoring the PCB is context switching, and this cost governs the choice of the round-robin (RR) time quantum (Week 6).

#### 🔗 Connection 6: MLFQ = A Practical Approximation of SJF

SJF is theoretically optimal but needs to know the future (Week 6). MLFQ approximates SJF **by observing past behavior** (Week 7). This is the foundation of modern schedulers.

#### 🔗 Connection 7: Fairness vs Efficiency, an Endless Trade-Off

- FCFS (simple) vs SJF (efficient): convoy effect vs starvation
- Affinity (performance) vs load balancing (fairness)
- RMS (simple) vs EDF (100% utilization)

**Every scheduling decision is a compromise between these two goals**. There is no universal answer, and the choice must fit the characteristics of the workload.

### 8.3 Advanced Tips for Writing Answers

#### Pattern 1: "Why" First

❌ "Dual mode has kernel/user modes."

✅ "With a single-mode CPU, a user program could control the disk directly and destroy the OS. Therefore the mode bit is kept in a hardware status register, and the hardware enforces the switch so that no software trick can bypass it."

#### Pattern 2: Mention the Trade-Off

❌ "In RR, the time quantum is important."

✅ "If the RR time quantum is too small, context switching cost dominates, and if it is too large, RR converges to FCFS and response time worsens. Empirically, a value at which 80% of CPU bursts complete within one quantum works well."

#### Pattern 3: Connect Concepts

❌ "Switching between threads is fast."

✅ "Context switching between threads is faster than between processes because **no TLB flush is needed**. Since the threads share the same address space, the TLB entries remain valid. This design connects directly to the address translation layer of virtual memory in Weeks 11-12."

#### Pattern 4: Limitations and Exceptions

❌ "SJF minimizes the average waiting time."

✅ "SJF minimizes the average waiting time in theory, but in practice the length of the next CPU burst cannot be known exactly, so it is predicted with exponential averaging τ = αt + (1-α)τ. Also, a long process can wait indefinitely (starvation), so aging is essential."

#### Pattern 5: Real-World Examples

❌ "Shared memory is a fast IPC."

✅ "A representative real-world use of shared memory is the shared buffer pool of PostgreSQL. Several connections map the same physical memory and avoid copying 8KB pages. Chromium's renderer↔GPU texture sharing also achieves zero copy in 4K rendering."

---

## Chapter 9. Last-Minute Exam Checklist

### 9.1 Thirty Questions You Must Be Able to Answer

#### Definitions and Basics (Week 1)
1. What is an OS? (kernel-centered)
2. What problems arise if there is no dual mode?
3. What is the difference between a system call and a library function?
4. What is the difference between a trap and a hardware interrupt?
5. What is the trade-off between monolithic and microkernel designs?

#### Processes (Week 2)
6. What is the difference between a process and a program?
7. Why is BSS separated from Data?
8. What are four main fields of the PCB?
9. Why does fork() return 0 to the child and the PID to the parent?
10. What are the advantages of designing fork and exec separately?
11. What is the difference between a zombie and an orphan, and how is each resolved?
12. Why is a context switch between threads faster than between processes?

#### IPC (Week 3)
13. What is the trade-off between shared memory and message passing?
14. Why does a bounded buffer store only SIZE−1 items?
15. Why must the unused ends of a pipe be closed?
16. When are exactly-once RPC semantics needed?
17. Why is marshalling needed (the endianness problem)?

#### Threads (Week 4)
18. What are the four benefits of multithreading?
19. What is the difference between concurrency and parallelism?
20. What does Amdahl's law imply?
21. Why did the one-to-one model become the standard?
22. Why do race conditions occur, and what is the principle behind the `partial_sum` array solution?

#### Implicit Threading (Week 5)
23. What are the three benefits of a thread pool?
24. How does OpenMP reduction avoid race conditions internally?
25. Why is calling exec() immediately after fork() safe (orphaned locks)?
26. How does TLS differ from local variables?

#### Basic Scheduling (Week 6)
27. Explain the convoy effect of FCFS with an example.
28. Why is SJF optimal, and what is its practical barrier?
29. What happens if the RR time quantum is too large or too small?

#### Advanced Scheduling (Week 7)
30. How does MLFQ approximate SJF and prevent starvation?

### 9.2 Calculation Practice

#### SJF Calculation 🧪
Processes: P1(6), P2(8), P3(7), P4(3), all arriving at t=0.

- Execution order: P4 → P1 → P3 → P2
- Waiting: P4=0, P1=3, P3=9, P2=16
- Average: 7

#### RR Calculation 🧪
Processes: P1(24), P2(3), P3(3), q=4, all arriving at t=0.

- Gantt: P1(0-4) P2(4-7) P3(7-10) P1(10-14) P1(14-18) P1(18-22) P1(22-26) P1(26-30)
- P1 waiting: 30-24=6, P2 waiting: 4, P3 waiting: 7
- Average: 5.67

#### Exponential Averaging 🧪
α=0.5, τ0=10. Actual burst t0=6.

$$ \tau_1 = 0.5 \cdot 6 + 0.5 \cdot 10 = 8 $$

Next actual burst t1 = 4:

$$ \tau_2 = 0.5 \cdot 4 + 0.5 \cdot 8 = 6 $$

#### Amdahl's Law 🧪
S = 20% (80% parallel), N = 8 cores.

$$ \text{speedup} = \frac{1}{0.2 + 0.8/8} = \frac{1}{0.3} \approx 3.33 $$

N→∞: 1/0.2 = **at most 5x**.

#### Little's Formula 🧪
In an interactive system, an average queue length of n=20 and a waiting time of W=0.5 seconds are observed.

$$ \lambda = n/W = 20/0.5 = 40 \text{ processes/s} $$

### 9.3 Code Pattern Practice

#### fork + exec + wait
```c
if ((pid = fork()) == 0) {
    execlp("ls", "ls", "-l", NULL);
    perror("exec"); exit(1);
} else if (pid > 0) {
    wait(NULL);
}
```

#### Pipe ls | wc -l
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

#### Safe Argument Passing to Pthreads
```c
int ids[N];
for (int i = 0; i < N; i++) {
    ids[i] = i;
    pthread_create(&t[i], NULL, f, &ids[i]);
}
```

#### OpenMP reduction
```c
int sum = 0;
#pragma omp parallel for reduction(+:sum)
for (int i = 0; i < N; i++) sum += a[i];
```

### 9.4 Exam Tips

#### Six-Element Answer Template
```text
[Definition in 1-2 sentences: Korean term (English term)]
+ [Why it is needed: alternatives/counterexamples]
+ [Mechanism in 2-4 sentences]
+ [One concrete example (code/calculation/practice)]
+ [Limitation/trade-off in 1 sentence]
+ [Link to another concept in 1 sentence]
```

#### Avoiding Common Mistakes
- [ ] fork() return value: 0 for the child, the PID for the parent (watch the order)
- [ ] Code after a successful exec(): never runs
- [ ] Pipe: close **all** unused ends (the parent too!)
- [ ] `write(fd, msg, strlen(msg)+1)`: include the null terminator
- [ ] pthread_create argument: `&i` from the loop is a race, so use `&ids[i]`
- [ ] Thread.start() vs run(): calling run() directly stays in the same thread
- [ ] SJF is optimal but needs prediction and risks starvation
- [ ] RR quantum: mention the 80% rule
- [ ] Mention Amdahl's upper bound 1/S
