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

This repository contains the lecture notes, coursework, and system-level code of an Operating Systems course on the design and implementation of operating systems, based on the Dinosaur Book (Operating System Concepts).

- `Lecture-Notes/` holds lecture notes on the concepts of each week's lectures and lab notes on the lab sessions of Weeks 1-6. The lecture slides were distributed by week, so the units are also numbered by week (`Week##`), and together they cover the full Dinosaur Book curriculum. Week 8, the midterm exam week, has no unit.
- Every lecture note and lab note comes as an English file (`.md`) and a Korean file (`.ko.md`) with the same structure.
- `Coursework/` holds the assignment solutions (C implementations, an xv6 kernel patch, and submitted reports) and midterm exam preparation materials. HW1 also comes with bilingual explanation documents (`.ko.md`, `.md`) and `.sh` grading scripts that check the solutions automatically.
- The code of the team-based term project and the lab environment xv6-riscv are linked under `Coursework/` as git submodules.

> **🤖 AI Agent Usage**
> This course encourages the use of AI agents.
> [Claude Code](https://claude.ai/download) and [Gemini CLI](https://github.com/google-gemini/gemini-cli) were used as coding assistants throughout the course, and Claude Code was used to restructure the repository.

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

- An understanding of computer architecture and C programming is required.
- A C compiler (e.g., GCC, Clang) must be installed.
- Familiarity with Unix/Linux shell environments is required.

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
