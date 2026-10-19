# [Spring 2026] Operating Systems

![Last Commit](https://img.shields.io/github/last-commit/Choroning/26Spring_Operating-Systems)
![Languages](https://img.shields.io/github/languages/top/Choroning/26Spring_Operating-Systems)

This repository archives bilingual lecture notes and coursework for the Operating Systems course.

*Author: Cheolwon Park (Korea University Sejong, CSE) - Year 3 (Junior) as of 2026*
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

- `Lecture-Notes/` holds a lecture note for each week (`Week##`), and Weeks 1-6 also hold lab notes.
- `Coursework/` holds the assignment solutions (C implementations, xv6 kernel patches, and reports) and midterm review materials, with the term project and xv6-riscv linked as git submodules.
- Every note comes as an English (`.md`) and Korean (`.ko.md`) pair.

> **🤖 AI Agent Usage**
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

This course covers the design and implementation of operating systems, including processes and threads, CPU scheduling, synchronization, memory management, file systems, and I/O. It combines theoretical foundations with implementation examples from Linux, UNIX, and xv6. The instructor and research lab, schedule and class format, assessment, course policies, weekly plan and progress, and references are in the [course information document](Course-Information.md).

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
├── Lecture-Notes               Lecture notes and lab notes by week
├── Coursework                  Assignments, exam preparation, term project, xv6 lab environment
├── images                      Figures for all documents
├── Course-Information.ko.md    Course operations
├── Course-Information.md
├── LICENSE                     Documentation license (CC BY-NC-SA 4.0)
├── LICENSE-CODE                Code license (PolyForm Noncommercial 1.0.0)
├── README.ko.md
└── README.md
```

The units are listed in the [`Lecture-Notes` index](Lecture-Notes/README.md), and the assignments and the term project in the [`Coursework` index](Coursework/README.md).

<br><a name="license"></a>
## 🤝 License

The documents in this repository (lecture notes, explanations, and summary sheets) are licensed under [CC BY-NC-SA 4.0](LICENSE), and the code under [PolyForm Noncommercial 1.0.0](LICENSE-CODE). Figures and example code taken from lecture slides and textbooks, as well as assignment handouts and template code distributed by the course, belong to their original authors and are not covered by these licenses.

---
