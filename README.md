# [Spring 2026] Operating Systems

![Last Commit](https://img.shields.io/github/last-commit/Choroning/26Spring_Operating-Systems)
![Languages](https://img.shields.io/github/languages/top/Choroning/26Spring_Operating-Systems)

This repository organizes and stores sample C code and shell scripts written for university lectures and assignments.

*Author: Cheolwon Park (Korea University Sejong, CSE) – Year 3 (Junior) as of 2026*
<br><br>

## 📑 Table of Contents

- [About This Repository](#about-this-repository)
- [Course Information](#course-information)
- [Prerequisites](#prerequisites)
- [Repository Structure](#repository-structure)
- [License](#license)

---


<br><a name="about-this-repository"></a>
## 📝 About This Repository

This repository contains bilingual study materials and system-level code developed for a university-level Operating Systems course, including:

- Bilingual Concepts notes (Korean `.ko.md` + English `.md`) for every lecture and lab session
- Assignment solutions with bilingual explanation documents (`.ko.md` + `.md`)
- C implementations with `.sh` grading scripts
- Weekly directory structure covering the full Dinosaur Book curriculum

> **🤖 AI-Assisted Development**
> This course encourages the use of AI agents.
> [Claude Code](https://claude.ai/download) and [Gemini CLI](https://github.com/google-gemini/gemini-cli) were used as coding assistants throughout the course.

<br><a name="course-information"></a>
## 📚 Course Information

- **Semester:** Spring 2026 (March - June)
- **Affiliation:** Korea University Sejong

| Course&nbsp;Code| Course            | Type          | Instructor      | Department                              |
|:----------:|:------------------|:-------------:|:---------------:|:----------------------------------------|
|`DCSS301-00`|OPERATING SYSTEM|Major Required|Prof. Unggi&nbsp;Lee|Department of Computer Science and Software Engineering|

### Course Overview

This course covers the design and implementation of operating systems, including processes and threads, CPU scheduling, synchronization, memory management, file systems, and I/O. It combines theoretical foundations with implementation examples from Linux, UNIX, and xv6.

### Instructor and Lab

- **Instructor:** Prof. Unggi Lee, Department of Computer Science and Software Engineering
- **Research lab:** [LEAP Lab](https://codingchild2424.github.io/lab-website/), studying generative AI in education, pedagogical alignment, large language models, and knowledge tracing
- **Hands-on lab:** xv6 for RISC-V, following [MIT 6.1810](https://pdos.csail.mit.edu/6.1810/)

### Schedule and Class Format

- **Credits:** 3
- **Meeting times:** Wednesday, periods 5–6; Thursday, period 8
- **Classroom:** Science and Technology Building 2, Room 310
- **Weekly format:** Period 1: lecture (part 1) and quiz; Period 2: lecture (part 2); Period 3: hands-on lab

### Assessment

| Component | Weight |
|:----------|-------:|
| Assignments (quizzes 5%, take-home assignments 5%) | 10% |
| Midterm exam (written) | 30% |
| Final exam (written) | 30% |
| Final project | 30% |
| Attendance | 0% |

- There are ten quizzes in Weeks 3–7 and 9–13, and five take-home assignments in Weeks 2–6.
- Written exams are handwritten, allow no electronic devices, and last one hour.
- The final project begins in Week 9. Teams of 3–4 design and develop an OS prototype, prepare a specification and project report, and present in person in Week 14. The project grade is split evenly between instructor and peer evaluation.
- Generative AI tools are permitted and encouraged for assignments and projects when students explain their own reasoning and design decisions.
- A grade is not awarded if a student misses more than one third of the total class hours.

### Course Roadmap

| Week | Topic | Week | Topic |
|:----:|:------|:----:|:------|
| 1 | Introduction | 9 | Synchronization Tools and Examples |
| 2 | Process 1 | 10 | Deadlocks |
| 3 | Process 2 | 11 | Main Memory |
| 4 | Thread and Concurrency 1 | 12 | Virtual Memory |
| 5 | Thread and Concurrency 2 | 13 | Storage Management, Security and Protection |
| 6 | CPU Scheduling 1 | 14 | Final Exam (Project) |
| 7 | CPU Scheduling 2 | 15 | Final Exam (Written) |
| 8 | Midterm Exam | 16 | Study Week |

- **📖 References**

| Type | Contents |
|:----:|:---------|
|Textbook|"Operating System Concepts, 10th Edition" by Silberschatz, Galvin, and Gagne (the Dinosaur Book)|
|Lecture Notes|[Instructor's Markdown notes and slides (GitHub)](https://github.com/codingchild2424/2026-lecture-operating-system)|

<br><a name="prerequisites"></a>
## ✅ Prerequisites

- Understanding of computer architecture and C programming
- C compiler (e.g., GCC, Clang) installed
- Familiarity with Unix/Linux shell environments

- **💻 Development Environment**

| Tool | Company |  OS  | Notes |
|:-----|:-------:|:----:|:------|
|Visual Studio Code|Microsoft|macOS|    |
|Xcode|Apple Inc.|macOS|    |

<br><a name="repository-structure"></a>
## 🗂 Repository Structure

```plaintext
26Spring_Operating-Systems
├── W01_Introduction-to-Operating-Systems
│   ├── Concepts_Lab.ko.md
│   ├── Concepts_Lab.md
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W02_Process-1
│   ├── Assignment
│   │   ├── minishell.c
│   │   ├── pingpong.c
│   │   ├── test_minishell.sh
│   │   └── test_pingpong.sh
│   ├── Assignment-Explanation.ko.md
│   ├── Assignment-Explanation.md
│   ├── Concepts_Lab.ko.md
│   ├── Concepts_Lab.md
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W03_Process-2
│   ├── Assignment
│   │   ├── trace_test.c
│   │   └── trace.patch
│   ├── Assignment-Report.pdf
│   ├── Concepts_Lab.ko.md
│   ├── Concepts_Lab.md
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W04_Thread-and-Concurrency-1
│   ├── Assignment
│   │   └── histogram.c
│   ├── Assignment-Report.pdf
│   ├── Concepts_Lab.ko.md
│   ├── Concepts_Lab.md
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W05_Thread-and-Concurrency-2
│   ├── Assignment
│   │   ├── matmul.c
│   │   └── mergesort.c
│   ├── Assignment-Report.pdf
│   ├── Concepts_Lab.ko.md
│   ├── Concepts_Lab.md
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W06_CPU-Scheduling-1
│   ├── Concepts_Lab.ko.md
│   ├── Concepts_Lab.md
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W07_CPU-Scheduling-2
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W09_Synchronization
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W10_Deadlocks
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W11_Main-Memory
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W12_Virtual-Memory
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W13_Storage-Management
│   ├── Concepts_Lecture.ko.md
│   └── Concepts_Lecture.md
├── W14_Security-Protection
├── Preparation
│   ├── Mid_SummarySheet.ko.md
│   └── Mid_Total.ko.md
├── images
│   ├── *.png                          (lecture diagrams and icons)
│   ├── cropped
│   │   └── (cropped figure images)
│   └── figures
│       └── (extracted lecture figures)
├── xv6-riscv                            (git submodule → mit-pdos/xv6-riscv)
├── LICENSE
├── README.ko.md
└── README.md
```

<br><a name="license"></a>
## 🤝 License

This repository is released under the [MIT License](LICENSE).

---
