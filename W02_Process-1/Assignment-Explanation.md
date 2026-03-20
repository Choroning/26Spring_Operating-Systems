# Week 2 Assignment Explanation — pingpong & minishell

> **Last Updated:** 2026-03-19
>
> Lecture reference: Lecture (Ch 3.1–3.3 Processes), Lab (Labs 3–4 pipe/dup2)

---

## File Structure and Usage

```plaintext
W02_Process-1/Assignment/
├── pingpong.c                  # Assignment 1 — A program where two processes exchange 1 byte via pipe
├── minishell.c                 # Assignment 2 — A simple shell supporting pipes and I/O redirection
├── test_pingpong.sh            # Automated grading script for pingpong.c
├── test_minishell.sh           # Automated grading script for minishell.c
└── Assignment-Explanation.md   # Assignment explanation document (this file)
```

### Build and Run

```bash
# pingpong
gcc -Wall -o pingpong pingpong.c
./pingpong

# minishell
gcc -Wall -o minishell minishell.c
./minishell
```

### Testing

```bash
# Run in the same directory (default: compiles .c files in the same path)
./test_pingpong.sh
./test_minishell.sh

# You can also specify a source file at a different path
./test_pingpong.sh /path/to/pingpong.c
./test_minishell.sh /path/to/minishell.c
```

---

## Table of Contents

- [1. Assignment Overview](#1-assignment-overview)
  - [1.1 System Calls Covered in This Assignment](#11-system-calls-covered-in-this-assignment)
- [2. Assignment 1 — pingpong.c Explanation](#2-assignment-1--pingpongc-explanation)
  - [2.1 What Are We Implementing?](#21-what-are-we-implementing)
  - [2.2 Why Are Two Pipes Needed?](#22-why-are-two-pipes-needed)
  - [2.3 TODO 1: Create Two Pipes](#23-todo-1-create-two-pipes)
  - [2.4 TODO 2, 6: Close Unused fds](#24-todo-2-6-close-unused-fds)
  - [2.5 TODO 3, 4, 5: Child Process Operations](#25-todo-3-4-5-child-process-operations)
  - [2.6 TODO 7, 8, 9: Parent Process Operations](#26-todo-7-8-9-parent-process-operations)
  - [2.7 Complete Data Flow Summary](#27-complete-data-flow-summary)
  - [2.8 Key Learning Points](#28-key-learning-points)
- [3. Assignment 2 — minishell.c Explanation (execute_command)](#3-assignment-2--minishellc-explanation-execute_command)
  - [3.1 What Are We Implementing?](#31-what-are-we-implementing)
  - [3.2 Understanding the Given Data Structure](#32-understanding-the-given-data-structure)
  - [3.3 TODO 1: Create a Child with fork()](#33-todo-1-create-a-child-with-fork)
  - [3.4 TODO 2a: Input Redirection](#34-todo-2a-input-redirection)
  - [3.5 TODO 2b: Output Redirection](#35-todo-2b-output-redirection)
  - [3.6 TODO 2c, 2d: Command Execution](#36-todo-2c-2d-command-execution)
  - [3.7 TODO 3: Parent Waiting](#37-todo-3-parent-waiting)
  - [3.8 execute_command Complete Flow Summary](#38-execute_command-complete-flow-summary)
- [4. Assignment 2 — minishell.c Explanation (execute_pipe)](#4-assignment-2--minishellc-explanation-execute_pipe)
  - [4.1 What Does a Pipe Do?](#41-what-does-a-pipe-do)
  - [4.2 TODO 4: Create a Pipe](#42-todo-4-create-a-pipe)
  - [4.3 TODO 5: Child 1 (Execute cmd1)](#43-todo-5-child-1-execute-cmd1)
  - [4.4 TODO 6: Child 2 (Execute cmd2)](#44-todo-6-child-2-execute-cmd2)
  - [4.5 TODO 7: Cleanup in the Parent](#45-todo-7-cleanup-in-the-parent)
  - [4.6 execute_pipe Complete Flow Summary](#46-execute_pipe-complete-flow-summary)
  - [4.7 Pipe + Redirection Combination](#47-pipe--redirection-combination)
- [5. Key Concepts Summary and Exam Preparation](#5-key-concepts-summary-and-exam-preparation)
  - [5.1 System Call Combination Patterns Summary](#51-system-call-combination-patterns-summary)
  - [5.2 fd Management Rules](#52-fd-management-rules)
  - [5.3 Connecting This Assignment to Lecture Content](#53-connecting-this-assignment-to-lecture-content)
  - [5.4 Exam Preparation — Frequently Tested Topics](#54-exam-preparation--frequently-tested-topics)
- [Summary](#summary)

---

<br>

## 1. Assignment Overview

### 1.1 System Calls Covered in This Assignment

| System Call | Role | Lecture Reference |
|:-----------|:-----|:-----------------|
| **fork()** | Create a copy (child process) of the current process | Lecture: Process Creation (Ch 3.3.1) |
| **exec()** | Replace the process image with a new program | Lecture: exec() after fork() |
| **wait()** | Wait for child process termination; prevent zombies | Lecture: Process Termination (Ch 3.3.2) |
| **pipe()** | Create a unidirectional IPC channel (kernel buffer) | Lab: Lab 3 — pipe() |
| **dup2()** | Duplicate a file descriptor (FD) for I/O redirection | Lab: Lab 4 — dup2() |
| **read() / write()** | Read/write data through file descriptors | Lab: Lab 3 — pipe() |
| **close()** | Close a file descriptor | Lab: Always close unused fds |

**Assignment Structure:**
- **pingpong** (basic): A program where two processes exchange 1 byte via pipes
- **minishell** (intermediate): A simple shell supporting pipes and I/O redirection

---

<br>

## 2. Assignment 1 — pingpong.c Explanation

### 2.1 What Are We Implementing?

**Goal:** The parent process and child process exchange 1 byte in a **round-trip** fashion through pipes, and measure the round-trip time (RTT).

```text
Parent Process                              Child Process
     │                                           │
     ├── Send 'p' (ping) ──────────────────────→ │
     │                                           ├── Print "received ping"
     │ ←──────────────────────── Send 'p' ────── │
     ├── Print "received pong"                   │
     ├── Print round-trip time                   │
     └── Reap child with wait()                  └── exit(0)
```

**Expected Output:**
```
12345: received ping
12344: received pong
Round-trip time: 156.000 us
```

> **[Computer Networks — RTT]** Round-trip Time (RTT) in networking refers to the time it takes for a packet to travel to a destination and back. In this assignment, we measure the RTT of inter-process pipe communication, which is useful for understanding IPC (Inter-Process Communication) overhead. Typically, pipe RTT within the same host is on the order of hundreds of microseconds.

### 2.2 Why Are Two Pipes Needed?

A pipe is a **unidirectional** communication channel. Data is written to one end (`fd[1]`) and read from the other end (`fd[0]`).

For **bidirectional communication**, two pipes are required:

```text
parent_to_child[2]:   Parent writes to [1] → Child reads from [0]
child_to_parent[2]:   Child writes to [1] → Parent reads from [0]
```

```c
int parent_to_child[2];   /* Parent → Child direction */
int child_to_parent[2];   /* Child → Parent direction */
```

| Pipe | Write end `[1]` | Read end `[0]` |
|:-----|:----------------|:---------------|
| `parent_to_child` | Used by parent | Used by child |
| `child_to_parent` | Used by child | Used by parent |

> **[Lab 3]** Recall from the lab that the `pipe()` function fills an `int fd[2]` array with `fd[0]` (read end) and `fd[1]` (write end). This assignment extends that principle to implement **bidirectional communication** using **two pipes**.

### 2.3 TODO 1: Create Two Pipes

```c
/* TODO 1: Create 2 pipes. */
if (pipe(parent_to_child) < 0) {
    perror("pipe");
    exit(1);
}
if (pipe(child_to_parent) < 0) {
    perror("pipe");
    exit(1);
}
```

**Key Points:**
- After calling `pipe()`, the return value must be checked for **error handling**.
- `pipe()` returns `-1` on failure and sets `errno`.
- `perror()` automatically prints the error message corresponding to `errno`.
- Pipes must be created **before** `fork()` — because the child process inherits the parent's file descriptor table.

> **[Lecture]** As learned in lecture, `fork()` creates a **copy of the parent process's address space**. The file descriptor table is also copied, so both parent and child will have the pipe fds created before `fork()`. This is why communication between parent and child through pipes is possible.

### 2.4 TODO 2, 6: Close Unused fds

After `fork()`, both parent and child hold 4 fds. Each process must close the **unused ends**.

**Child Process (TODO 2):**
```c
/* The child only reads from parent_to_child and only writes to child_to_parent */
close(parent_to_child[1]);   /* Write end of parent→child pipe (child doesn't write) */
close(child_to_parent[0]);   /* Read end of child→parent pipe (child doesn't read) */
```

**Parent Process (TODO 6):**
```c
/* The parent only writes to parent_to_child and only reads from child_to_parent */
close(parent_to_child[0]);   /* Read end of parent→child pipe (parent doesn't read) */
close(child_to_parent[1]);   /* Write end of child→parent pipe (parent doesn't write) */
```

> **[Lab 3]** In the lab, we learned the rule "always close the unused end." The reasons: (1) If the write end remains open, the reading side **cannot receive EOF**, causing the program to block forever. (2) File descriptors are a finite resource, so unnecessary fds should be released immediately.

### 2.5 TODO 3, 4, 5: Child Process Operations

```c
/* TODO 3: Read 1 byte from the parent. */
char buf;
read(parent_to_child[0], &buf, 1);    /* Read 'p' sent by the parent */
printf("%d: received ping\n", getpid());

/* TODO 4: Send 1 byte to the parent. */
write(child_to_parent[1], &buf, 1);   /* Send the received byte back to the parent */

/* TODO 5: Close the used pipe fds and terminate with exit(0). */
close(parent_to_child[0]);
close(child_to_parent[1]);
exit(0);
```

**Execution Flow:**
1. `read()` **blocks** until the parent sends data — serves as synchronization
2. After reading the byte, print `"received ping"`
3. Send the same byte back to the parent via `write()` (pong)
4. Close all used fds and terminate normally with `exit(0)`

> **[Lecture]** When a child process terminates with `exit(0)`, it becomes a **zombie process** until the parent calls `wait()`. In this assignment, the parent calls `wait(NULL)` at the end to reap the zombie.

### 2.6 TODO 7, 8, 9: Parent Process Operations

```c
long start_time = get_time_us();      /* Start time measurement */

/* TODO 7: Send 1 byte to the child (ping). */
char buf = 'p';
write(parent_to_child[1], &buf, 1);   /* Send 'p' */

/* TODO 8: Read 1 byte from the child (pong). */
read(child_to_parent[0], &buf, 1);    /* Read the child's response */
printf("%d: received pong\n", getpid());

long end_time = get_time_us();        /* End time measurement */
double elapsed = (double)(end_time - start_time);
printf("Round-trip time: %.3f us\n", elapsed);

/* TODO 9: Close the used pipe fds and wait for the child with wait(). */
close(parent_to_child[1]);
close(child_to_parent[0]);
wait(NULL);   /* Reap child process — prevent zombie */
```

> **[Lecture — Context Switch]** The round-trip time includes **context switch** overhead. For the child to wake up on `read()` after the parent calls `write()`, the CPU must switch from parent to child, and for the parent to wake up on `read()` after the child calls `write()`, another switch is needed. In other words, measured RTT = data copy time + context switch time x 2.

### 2.7 Complete Data Flow Summary

```text
Time →

Parent Process                                 Child Process
━━━━━━━━━━━━━━                                ━━━━━━━━━━━━━
pipe(parent_to_child)
pipe(child_to_parent)
fork() ──────────────────────────────────────→ [Created]
close(parent_to_child[0])              close(parent_to_child[1])
close(child_to_parent[1])              close(child_to_parent[0])
                                                    │
start_time = get_time_us()                          │
write(parent_to_child[1], 'p') ────→ read(parent_to_child[0])
        │                            printf("received ping")
        │                         write(child_to_parent[1], 'p')
read(child_to_parent[0]) ←──────────
printf("received pong")                       close(...)
end_time = get_time_us()                 exit(0) → [Zombie state]
printf("Round-trip time: ...")                  │
close(...)                                      │
wait(NULL) ──────────────────────────────→ [Reaped]
```

### 2.8 Key Learning Points

| Learning Point | Lecture Reference | Application in This Assignment |
|:--------------|:-----------------|:------------------------------|
| **Pipes are unidirectional** | Lab: `pipe()` exercise | Two pipes used for bidirectional communication |
| **fork() inherits fds** | Lecture: How fork() works | Create pipe() before fork() → both parent and child hold fds |
| **Close unused fds** | Lab: Pipe rules | close() 2 unnecessary fds in each process |
| **read() blocks** | Lab: Producer-consumer pattern | Child's read() waits until parent writes → natural synchronization |
| **Prevent zombies with wait()** | Lecture: Zombie processes | Parent calls wait(NULL) at the end |
| **Context switch overhead** | Lecture: Context switch | RTT includes context switch cost |

---

<br>

## 3. Assignment 2 — minishell.c Explanation (execute_command)

### 3.1 What Are We Implementing?

**Goal:** Implement the core behavior of an **actual shell** using `fork()`, `exec()`, `wait()`, `pipe()`, and `dup2()`.

| Feature | Example | System Calls Used |
|:--------|:--------|:-----------------|
| Single command execution | `ls -l` | fork + execvp + wait |
| Output redirection | `echo hello > file.txt` | open + dup2 + execvp |
| Input redirection | `sort < data.txt` | open + dup2 + execvp |
| Pipe | `ls \| wc -l` | pipe + fork×2 + dup2 + execvp |
| Exit | `exit` | Loop exit |

**Already implemented:** `main()` (input reading, pipe splitting, exit handling), `parse_command()` (command parsing, redirection parsing), `struct command`

**To be implemented by the student:** `execute_command()`, `execute_pipe()`

> **[Lecture — How Shells Work]** As taught in lecture, "the basic operating principle of a UNIX shell = repeated fork() + exec() + wait()." This assignment directly implements that principle in code.

### 3.2 Understanding the Given Data Structure

```c
struct command {
    char *argv[MAX_ARGS];   /* Argument list (NULL-terminated) */
    char *infile;           /* Input redirection filename, NULL if none */
    char *outfile;          /* Output redirection filename, NULL if none */
};
```

**Parsing Examples:**

| Input | `argv[]` | `infile` | `outfile` |
|:------|:---------|:---------|:----------|
| `ls -l` | `{"ls", "-l", NULL}` | `NULL` | `NULL` |
| `echo hello > out.txt` | `{"echo", "hello", NULL}` | `NULL` | `"out.txt"` |
| `sort < data.txt` | `{"sort", NULL}` | `"data.txt"` | `NULL` |
| `cat < in.txt > out.txt` | `{"cat", NULL}` | `"in.txt"` | `"out.txt"` |

The `parse_command()` function recognizes `<` and `>` as tokens, stores them in `infile`/`outfile`, and places the rest in `argv[]` — this is already implemented.

### 3.3 TODO 1: Create a Child with fork()

```c
static int execute_command(struct command *cmd)
{
    /* TODO 1: Create a child process with fork(). */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    ...
}
```

**Why is fork() necessary?**
- The shell itself must continue running (to accept the next command).
- `exec()` **overwrites** the current process image, so it must run in a child to keep the shell alive.
- This is the core reason for the **fork() + exec()** pattern.

> **[Lecture — fork() Return Values]** The fork() return value rules taught in lecture are directly applied here. The parent receives a positive value (child pid) to use with `waitpid()`, and the child receives 0 to branch into the `exec()` path. `-1` indicates fork failure (e.g., process table full), which must be handled appropriately.

### 3.4 TODO 2a: Input Redirection

```c
if (pid == 0) {
    /* TODO 2 (Child Process): */

    /* a. Input redirection */
    if (cmd->infile != NULL) {
        int fd = open(cmd->infile, O_RDONLY);   /* Open file as read-only */
        if (fd < 0) {
            perror(cmd->infile);
            exit(1);                            /* Exit on file open failure */
        }
        dup2(fd, STDIN_FILENO);                 /* Replace stdin (fd 0) with the file */
        close(fd);                              /* Close the original fd */
    }
    ...
}
```

**How `dup2(fd, STDIN_FILENO)` works:**
1. In the file descriptor table, change what `STDIN_FILENO` (= 0) points to so it is the same as `fd`
2. When the program executed via `exec()` reads from `stdin`, it actually reads from the file
3. The program is **unaware** of the redirection — it simply reads from stdin

> **[Lab 4]** When you run `sort < data.txt`, the `sort` program reads from `stdin` as usual, but the actual content read comes from `data.txt`. This is why UNIX I/O redirection is powerful — you can change the input/output target without modifying the program.

### 3.5 TODO 2b: Output Redirection

```c
    /* b. Output redirection */
    if (cmd->outfile != NULL) {
        int fd = open(cmd->outfile,
                      O_WRONLY | O_CREAT | O_TRUNC,   /* Write-only | Create if absent | Overwrite if exists */
                      0644);                          /* Permissions: rw-r--r-- */
        if (fd < 0) {
            perror(cmd->outfile);
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);    /* Replace stdout (fd 1) with the file */
        close(fd);
    }
```

**`open()` flag meanings:**

| Flag | Meaning |
|:-----|:--------|
| `O_WRONLY` | Open as write-only |
| `O_CREAT` | Create the file if it does not exist |
| `O_TRUNC` | If the file exists, truncate it and write from the beginning (shell `>` behavior) |

**Permissions `0644`**: Owner can read+write, group and others can only read

### 3.6 TODO 2c, 2d: Command Execution

```c
    /* c. Execute the command */
    execvp(cmd->argv[0], cmd->argv);

    /* d. exec failure — reaching this line means exec failed */
    perror("minishell");
    exit(127);
}
```

**Why `execvp()` is used:**
- `v` (vector): Arguments are passed as a `char *argv[]` array — the `argv` from `struct command` can be used directly
- `p` (path): Automatically searches for the executable in the `PATH` environment variable — no need to specify the full path (`/bin/ls`), just `ls` is sufficient

**When exec succeeds:**
- The child process's text, data, heap, and stack are **completely replaced** with the new program
- Code after `execvp()` is **never executed**

**When exec fails (e.g., non-existent command):**
- `execvp()` returns and the code below it executes
- `exit(127)`: By shell convention, "command not found" uses exit code 127

> **[Lecture — exec() Family]** Among the exec() family functions taught in lecture (execl, execlp, execle, execv, execvp, execve), `execvp` is chosen because the arguments are in array form (`argv`) and PATH searching is needed. l/v determines the argument passing method, p determines PATH searching, and e determines environment variable specification.

### 3.7 TODO 3: Parent Waiting

```c
    /* TODO 3 (Parent Process): */
    int status;
    waitpid(pid, &status, 0);       /* Wait for specific child (pid) to terminate */
    if (WIFEXITED(status))
        return WEXITSTATUS(status); /* Return exit code on normal termination */
    return -1;                      /* Abnormal termination */
}
```

**`waitpid()` vs `wait()`:**

| Function | Target | When to Use |
|:---------|:-------|:-----------|
| `wait(NULL)` | Any one child | Convenient when there is only 1 child |
| `waitpid(pid, &status, 0)` | Specific child by pid | Precise control when there are multiple children |

Here there is only 1 child, but `waitpid()` is used to receive the termination status of **exactly that child**.

**Termination Status Macros:**
- `WIFEXITED(status)` — Check whether the child terminated normally via `exit()`
- `WEXITSTATUS(status)` — Extract the exit code (0–255)

### 3.8 execute_command Complete Flow Summary

```text
execute_command(cmd) called — e.g., "ls -l > out.txt"
    │
    ├── fork()
    │     │
    │     ├── [Child Process]
    │     │     ├── If infile exists: open() → dup2(fd, STDIN) → close(fd)
    │     │     ├── If outfile exists: open() → dup2(fd, STDOUT) → close(fd)
    │     │     ├── execvp("ls", {"ls", "-l", NULL})
    │     │     │     → Success: process image replaced with ls
    │     │     │     → ls outputs to stdout (= out.txt)
    │     │     │     → ls terminates with exit()
    │     │     └── On exec failure: perror + exit(127)
    │     │
    │     └── [Parent Process]
    │           ├── waitpid(pid, &status, 0)  ← blocks until child terminates
    │           └── Returns WEXITSTATUS(status)
    │
    └── Returns to shell's main() → prints next command prompt
```

This is the concrete implementation of the **"fork() + exec() + wait() = shell's basic operation"** pattern taught in lecture.

---

<br>

## 4. Assignment 2 — minishell.c Explanation (execute_pipe)

### 4.1 What Does a Pipe Do?

When you type `cmd1 | cmd2` in a shell:
- `cmd1`'s **standard output (stdout)** is connected to `cmd2`'s **standard input (stdin)** through a pipe.

```text
Example: ls | wc -l

   ls process                  Pipe (kernel buffer)           wc -l process
━━━━━━━━━━━━━━━            ━━━━━━━━━━━━━━━━━━━            ━━━━━━━━━━━━━━━
    stdout ──────────────→ [   kernel buffer   ] ──────────────→ stdin
 (outputs file list)                                          (counts lines)
```

**What execute_pipe() must do:**
1. Create 1 pipe (cmd1 → cmd2 direction)
2. Create 2 children (one for cmd1, one for cmd2)
3. In each child, connect the pipe to stdin/stdout using `dup2()`
4. In the parent, close the pipe fds + `wait()` for both children

> **[Lab 4]** The lab included a practice exercise for implementing `ls | wc -l`. The `execute_pipe()` in this assignment is a generalization of that implementation, allowing any two commands to be connected via a pipe.

### 4.2 TODO 4: Create a Pipe

```c
static int execute_pipe(struct command *cmd1, struct command *cmd2)
{
    /* TODO 4: Create a pipe with pipe(). */
    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return -1;
    }
    ...
}
```

**Difference from pingpong:**
- pingpong: Bidirectional communication → **2** pipes
- minishell pipe: Unidirectional communication (cmd1 → cmd2) → **1** pipe

```text
fd[0] = read end  → will be connected to cmd2's stdin
fd[1] = write end → will be connected to cmd1's stdout
```

### 4.3 TODO 5: Child 1 (Execute cmd1)

```c
/* TODO 5: Create child 1 with fork() (to execute cmd1). */
pid_t pid1 = fork();
if (pid1 < 0) {
    perror("fork");
    close(fd[0]);
    close(fd[1]);    /* Clean up pipe on error */
    return -1;
}

if (pid1 == 0) {
    /* Child 1: Connect cmd1's stdout to pipe write end */
    close(fd[0]);                      /* Read end not used */
    dup2(fd[1], STDOUT_FILENO);        /* stdout → pipe write end */
    close(fd[1]);                      /* Close original fd after dup2 */

    if (cmd1->infile != NULL) {        /* If cmd1 has input redirection */
        int infd = open(cmd1->infile, O_RDONLY);
        if (infd < 0) { perror(cmd1->infile); exit(1); }
        dup2(infd, STDIN_FILENO);
        close(infd);
    }

    execvp(cmd1->argv[0], cmd1->argv); /* Execute cmd1 */
    perror("minishell");
    exit(127);
}
```

**Key Point:** `dup2(fd[1], STDOUT_FILENO)` — when cmd1 outputs via `printf()` or `write(1,...)`, the data goes into the **pipe's kernel buffer**.

### 4.4 TODO 6: Child 2 (Execute cmd2)

```c
/* TODO 6: Create child 2 with fork() (to execute cmd2). */
pid_t pid2 = fork();
if (pid2 < 0) {
    perror("fork");
    close(fd[0]);
    close(fd[1]);
    return -1;
}

if (pid2 == 0) {
    /* Child 2: Connect cmd2's stdin to pipe read end */
    close(fd[1]);                      /* Write end not used */
    dup2(fd[0], STDIN_FILENO);         /* stdin → pipe read end */
    close(fd[0]);                      /* Close original fd after dup2 */

    if (cmd2->outfile != NULL) {       /* If cmd2 has output redirection */
        int outfd = open(cmd2->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outfd < 0) { perror(cmd2->outfile); exit(1); }
        dup2(outfd, STDOUT_FILENO);
        close(outfd);
    }

    execvp(cmd2->argv[0], cmd2->argv); /* Execute cmd2 */
    perror("minishell");
    exit(127);
}
```

**Key Point:** `dup2(fd[0], STDIN_FILENO)` — when cmd2 reads via `scanf()` or `read(0,...)`, the data is read from the **pipe's kernel buffer**. In other words, cmd1's output becomes cmd2's input.

### 4.5 TODO 7: Cleanup in the Parent

```c
/* TODO 7: Clean up in the parent. */
close(fd[0]);                    /* Close parent's pipe fds (very important!) */
close(fd[1]);

int status;
waitpid(pid1, NULL, 0);          /* Wait for child 1 (cmd1) to terminate */
waitpid(pid2, &status, 0);       /* Wait for child 2 (cmd2) to terminate + collect status */

if (WIFEXITED(status))
    return WEXITSTATUS(status);  /* Return cmd2's exit code (shell convention) */
return -1;
```

**Why is it important for the parent to close both ends of the pipe?**

- The parent also holds `fd[0]` and `fd[1]` created before `fork()`.
- If the parent does not close `fd[1]` (write end):
  - cmd2's `read()` will **never receive EOF** → blocks forever → program hangs!
- UNIX pipe rule: **All write ends must be closed** for the read side to receive EOF

> **[Lab 3]** This is the practical application of the rule learned in the lab: "closing the write end → the read side receives EOF." Since the parent, child 1, and child 2 — a total of 3 processes — all hold pipe fds, **all unused fds must be closed in each process** for correct operation.

### 4.6 execute_pipe Complete Flow Summary

```text
execute_pipe(cmd1, cmd2) called — e.g., "ls | wc -l"
    │
    ├── pipe(fd)  → fd[0]=read, fd[1]=write
    │
    ├── fork() → Child 1 (cmd1 = "ls")
    │     ├── close(fd[0])                 /* Read end not used */
    │     ├── dup2(fd[1], STDOUT_FILENO)   /* stdout → pipe */
    │     ├── close(fd[1])
    │     └── execvp("ls", {"ls", NULL})
    │           → ls output goes into the pipe
    │
    ├── fork() → Child 2 (cmd2 = "wc -l")
    │     ├── close(fd[1])                 /* Write end not used */
    │     ├── dup2(fd[0], STDIN_FILENO)    /* stdin ← pipe */
    │     ├── close(fd[0])
    │     └── execvp("wc", {"wc", "-l", NULL})
    │           → wc reads from pipe and counts lines
    │
    ├── [Parent] close(fd[0]), close(fd[1])  /* Critical! Ensures EOF delivery */
    ├── waitpid(pid1, ...)
    └── waitpid(pid2, &status, ...)
```

### 4.7 Pipe + Redirection Combination

This implementation also supports **combining pipes with redirection**:

```bash
minishell> echo hello | cat > /tmp/pipe_test.txt
```

In this case:
- **cmd1** = `echo hello` → stdout connected to **pipe**
- **cmd2** = `cat > /tmp/pipe_test.txt` → stdin from **pipe**, stdout to **file**

```text
echo hello                  Pipe                       cat
━━━━━━━━━━                ━━━━━━                    ━━━━
stdout → [pipe] → stdin
                                                  stdout → /tmp/pipe_test.txt
```

In the code, child 2 checks `cmd2->outfile` to additionally set up output redirection, which handles this case.

---

<br>

## 5. Key Concepts Summary and Exam Preparation

### 5.1 System Call Combination Patterns Summary

| Pattern | Purpose | Application in This Assignment |
|:--------|:--------|:------------------------------|
| **fork() + wait()** | Create child and wait for termination | pingpong: parent waits for child |
| **fork() + exec() + wait()** | Execute a new program (shell's basic operation) | minishell: execute_command() |
| **pipe() + fork() + read/write** | Exchange data between processes | pingpong: 1-byte round-trip |
| **pipe() + fork()x2 + dup2() + exec()** | Implement shell pipeline | minishell: execute_pipe() |
| **open() + dup2() + close()** | I/O redirection | minishell: input/output redirection |

### 5.2 fd Management Rules

**Rule 1: Always close unused pipe fds**
- If not closed, EOF will not be delivered and the program will block forever

**Rule 2: Close the original fd after `dup2()`**
- After `dup2(fd, STDOUT_FILENO)`, call `close(fd)` — since both fd and STDOUT point to the same file, the original fd is unnecessary

**Rule 3: After fork(), all 3 processes (parent, child 1, child 2) hold pipe fds**
- Unnecessary fds must be closed in each process.

```text
fd ownership (after fork()):

               fd[0]  fd[1]
     Parent:     ✕      ✕     ← close both
Child 1(cmd1):   ✕      ✓     ← close fd[0], connect fd[1] to stdout then close
Child 2(cmd2):   ✓      ✕     ← close fd[1], connect fd[0] to stdin then close
```

### 5.3 Connecting This Assignment to Lecture Content

**Theory (Lecture) Connections:**

| Lecture Concept | What You Directly Experienced in This Assignment |
|:---------------|:------------------------------------------------|
| Process = running program | After `fork()`, the child process becomes an independent execution unit |
| Process creation (parent→child) | Created child with `fork()`, verified relationship with `getpid()`/`getppid()` |
| fork() return values | Parent gets child pid, child gets 0 — branched with if-else |
| Loading a new program with exec() | Replaced child's memory image with `execvp()` |
| Zombie process | Occurs if `wait()`/`waitpid()` is not called |
| Context switch | pingpong's RTT includes context switch overhead |
| fork() + exec() + wait() = shell | minishell's execute_command() is exactly this pattern |

**Lab Connections:**

| Lab Concept | What You Directly Experienced in This Assignment |
|:-----------|:------------------------------------------------|
| pipe() is unidirectional | Used 2 pipes for bidirectional communication in pingpong |
| Close unused fds | Called close() on unnecessary fds in every process |
| I/O redirection with dup2() | Connected files/pipes to stdin/stdout in minishell |
| Producer-consumer pattern | Data flow from cmd1→cmd2 through pipes |

### 5.4 Exam Preparation — Frequently Tested Topics

**1. fork() Related:**
- fork() return values (parent: child pid, child: 0, failure: -1)
- After fork(), the execution order of parent and child is **not guaranteed** (scheduler-dependent)
- When fork() is called, the parent's address space is **copied** (including fd table)

**2. exec() Related:**
- On exec() success, it **does not return** to the original code (memory is overwritten)
- exec() does not change the PID

**3. wait() Related:**
- Not calling wait() → **zombie process**
- If the parent terminates first → **orphan process** → adopted by init/systemd

**4. pipe() Related:**
- Pipes are **unidirectional** — bidirectional requires 2
- All write ends must be closed for the read side to receive **EOF**
- Failing to close unused fds can cause **deadlock**

**5. dup2() Related:**
- `dup2(oldfd, newfd)` — makes newfd point to the same target as oldfd
- Shell's `>`, `<`, `|` are all implemented using dup2()

> **Exam Tip:** You must understand the operating principles of the above 5 system calls precisely, and master the order in which they must be called in code (pipe before fork, dup2 after fork, set up redirection before exec, etc.).

---

<br>

## Summary

| Concept | Key Summary |
|:--------|:-----------|
| pingpong.c | `pipe()` (bidirectional = 2) + `fork()` + `read()`/`write()` + `wait()`; inter-process synchronization and RTT measurement via pipes |
| fd management | Always close() unused fds; close original fd after dup2(); clean up unnecessary fds in all processes after fork() |
| execute_command() | `fork()` + `exec()` + `wait()` = shell's basic operating principle; `open()` + `dup2()` + `close()` = I/O redirection |
| execute_pipe() | `pipe()` + `fork()x2` + `dup2()` = shell pipeline; close unnecessary fds in all 3 processes including parent |
| Why execvp() | `v`(array args) + `p`(PATH search) — uses struct command's argv directly with automatic PATH searching |
| Pipe + redirection combo | cmd1's stdout → pipe → cmd2's stdin; cmd2 can additionally have output redirection |
| System call combination patterns | fork+wait, fork+exec+wait, pipe+fork+read/write, pipe+fork×2+dup2+exec, open+dup2+close |
| Exam essentials | Distinguishing fork return values, exec does not return on success, zombie on missing wait, unidirectional pipe principle, dup2 operating principle |
| Lecture connections | Lecture: process creation/termination/zombie/context switch (Ch 3.1–3.3); Lab: pipe/dup2/fd management (Labs 3–4) |

---
