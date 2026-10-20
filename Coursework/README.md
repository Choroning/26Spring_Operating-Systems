# Coursework

[Repository Guide](../README.md) | [English](README.md) | [한국어](README.ko.md)

> **Last Updated:** 2026-10-20

| Item | Description | Documents |
|:-----|:-----|:-----|
| [Exam_Midterm](Exam_Midterm/) | Midterm summary sheet and review covering Weeks 1-7 (introduction to operating systems, processes, IPC, threads and concurrency, CPU scheduling) | [Summary sheet](Exam_Midterm/Summary-Sheet.md), [Comprehensive review](Exam_Midterm/Total-Review.md) |
| [HW01_Process-1](HW01_Process-1/) | Week 2 take-home assignment. It implements `pingpong.c`, in which two processes exchange one byte through a pipe, and `minishell.c`, a simple shell that supports pipes and I/O redirection, with `fork()`, `exec()`, `wait()`, `pipe()`, and `dup2()`, and connects to Week 2 (Process 1) | [Assignment explanation](HW01_Process-1/Assignment-Explanation.md), [Solution and grading scripts](HW01_Process-1/Solution/) |
| [HW02_Process-2](HW02_Process-2/) | Week 3 take-home assignment. A kernel patch and a test program that add the `trace` and `sysinfo` system calls to xv6, connecting to Week 3 (Process 2) | [Report](HW02_Process-2/Solution/Assignment-Report.pdf), [Solution](HW02_Process-2/Solution/) |
| [HW03_Thread-and-Concurrency-1](HW03_Thread-and-Concurrency-1/) | Week 4 take-home assignment. It implements a parallel histogram with Pthreads data parallelism and measures the speedup by thread count, and connects to Week 4 (Thread and Concurrency 1) | [Report](HW03_Thread-and-Concurrency-1/Solution/Assignment-Report.pdf), [Solution](HW03_Thread-and-Concurrency-1/Solution/) |
| [HW04_Thread-and-Concurrency-2](HW04_Thread-and-Concurrency-2/) | Week 5 take-home assignment. It implements a parallel merge sort with Pthreads fork-join and a matrix multiplication with OpenMP, and connects to implicit threading in Week 5 (Thread and Concurrency 2) | [Report](HW04_Thread-and-Concurrency-2/Solution/Assignment-Report.pdf), [Solution](HW04_Thread-and-Concurrency-2/Solution/) |
| [HW05_CPU-Scheduling-1](HW05_CPU-Scheduling-1/) | Week 6 take-home assignment. A kernel patch and a test program that add the `setpriority` and `getpriority` system calls to xv6 and implement a priority scheduler that runs the process with the highest priority (0-20, a lower value is higher, default 10) and round-robins among processes of equal priority, connected to Week 6 (CPU Scheduling 1) | [Report](HW05_CPU-Scheduling-1/Solution/Assignment-Report.pdf), [Solution](HW05_CPU-Scheduling-1/Solution/) |
| [Project_LLM-Operating-System](Project_LLM-Operating-System/) | A web-based operating system for LLM applications developed by Team Garbage Collector as a team-based term project. The current version `GarbageCollector_GCOS` and the legacy version `GarbageCollector_OS` are linked as git submodules | [GarbageCollector_GCOS](https://github.com/Choroning/GarbageCollector_GCOS), [GarbageCollector_OS](https://github.com/Choroning/GarbageCollector_OS) |
| xv6-riscv | The RISC-V version of MIT's teaching operating system xv6, used across the labs and assignments. It is linked as a git submodule | [mit-pdos/xv6-riscv](https://github.com/mit-pdos/xv6-riscv) |
