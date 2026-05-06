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
├── W11_Main-Memory
├── W12_Virtual-Memory
├── W13_Storage-Management
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
