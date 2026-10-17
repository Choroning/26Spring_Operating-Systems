# Course Information

[Repository Guide](README.md) | [English](Course-Information.md) | [한국어](Course-Information.ko.md)

> **Last Updated:** 2026-10-18

## Table of Contents

- [1. Instructor and Research Lab](#1-instructor-and-research-lab)
- [2. Schedule and Class Format](#2-schedule-and-class-format)
- [3. Assessment](#3-assessment)
- [4. Course Policies](#4-course-policies)
- [5. Weekly Plan and Progress](#5-weekly-plan-and-progress)
- [6. References](#6-references)

---

<br>

## 1. Instructor and Research Lab

- **Instructor:** Prof. Unggi Lee, Department of Computer Science and Software Engineering
- **Research Lab:** [LEAP Lab](https://codingchild2424.github.io/lab-website/), studying generative AI in education, pedagogical alignment, large language models, and knowledge tracing.

---

<br>

## 2. Schedule and Class Format

- **Credits:** 3
- **Class Times:** Wednesday periods 5-6, Thursday period 8
- **Classroom:** Science and Technology Building 2, Room 310
- **Weekly Format:** Period 1: lecture (part 1) and quiz; Period 2: lecture (part 2); Period 3: hands-on lab
- **Lab Environment:** RISC-V xv6, using [MIT 6.1810](https://pdos.csail.mit.edu/6.1810/) resources.

---

<br>

## 3. Assessment

| Item | Weight |
|:-----|-----:|
| Assignments (quizzes 5%, take-home assignments 5%) | 10% |
| Midterm Exam (written) | 30% |
| Final Exam (written) | 30% |
| Term Project | 30% |
| Attendance | 0% |

- There are ten quizzes in Weeks 3-7 and 9-13, and five take-home assignments in Weeks 2-6.
- Written exams are handwritten, allow no electronic devices, and last one hour.
- The team-based term project begins in Week 9. Teams of 3-4 design and develop an OS prototype, prepare a specification and project report, and present in person in Week 14.
- The term project grade is split evenly between instructor and peer evaluation.

---

<br>

## 4. Course Policies

- Generative AI tools are permitted and encouraged for assignments and the term project. Students must explain their own reasoning and design decisions.
- A grade is not awarded if a student misses more than one third of the total class hours.

---

<br>

## 5. Weekly Plan and Progress

| Week | Class Dates (Wed, Thu) | Planned Topic | Lecture Decks Covered |
|:-----|:-----|:-----|:-----|
| 1 | 3/4, 3/5 | Introduction | W01_LEC_Introduction<br>W01_LAB_Coding-Agents |
| 2 | 3/11, 3/12 | Process 1 | W02_LEC_Process-1<br>W02_LAB_Process-System-Calls<br>HW1 |
| 3 | 3/18, 3/19 | Process 2 | W03_LEC_Process-2<br>W03_LAB_Exploring-XV6-Internals<br>HW2 |
| 4 | 3/25, 3/26 | Thread and Concurrency 1 | W04_LEC_Thread-And-Concurrency-1<br>W04_LAB_Pthread-Basics<br>HW3 |
| 5 | 4/1, 4/2 | Thread and Concurrency 2 | W05_LEC_Thread-And-Concurrency-2<br>W05_LAB_Implicit-Threading-And-Threading-Issues<br>HW4 |
| 6 | 4/8, 4/9 | CPU Scheduling 1 | W06_LEC_CPU-Scheduling-1<br>W06_LAB_Context-Switching<br>HW5 |
| 7 | 4/15, 4/16 | CPU Scheduling 2 | W07_LEC_CPU-Scheduling-2 |
| 8 | 4/22, 4/23 | Midterm Exam | Midterm exam |
| 9 | 4/29, 4/30 | Synchronization Tools and Examples | W09_LEC_Synchronization<br>W09_LAB_Sleep-Wakeup-And-Pipes<br>Term project starts |
| 10 | 5/6, 5/7 | Deadlocks | W10_LEC_Deadlocks<br>W10_LAB_Deadlocks |
| 11 | 5/13, 5/14 | Main Memory | W11_LEC_Main-Memory<br>W11_LAB_Page-Tables |
| 12 | 5/20, 5/21 | Virtual Memory | W12_LEC_Virtual-Memory<br>W12_LAB_COW-Fork-And-Lazy-Allocation |
| 13 | 5/27, 5/28 | Storage Management, Security and Protection | W13_LEC_Storage-Management<br>W13_LAB_File-System-Internals |
| 14 | 6/3, 6/4 | Final Exam (Term Project) | W14_LEC_Security-And-Protection |
| 15 | 6/10, 6/11 | Final Exam (Written) | - |
| 16 | 6/17, 6/18 | Study Week | - |

- Each week's slides were distributed as a lecture deck (`W##_LEC_`) and a lab deck (`W##_LAB_`), and they are organized in the `Week##` unit of the same number as `Concepts_Lecture` and `Concepts_Lab`, respectively. The units are listed in the [`Lecture-Notes` index](Lecture-Notes/README.md).
- Weeks 7 and 14 have no lab deck, and no lab notes were made for the lab decks of Weeks 9-13 (`W09_LAB_Sleep-Wakeup-And-Pipes`, `W10_LAB_Deadlocks`, `W11_LAB_Page-Tables`, `W12_LAB_COW-Fork-And-Lazy-Allocation`, `W13_LAB_File-System-Internals`).
- Security and protection was planned for Week 13, but its slides were distributed as a Week 14 deck (`W14_LEC_Security-And-Protection`), so it is organized in the `Week14` unit.
- HW1-HW4 are the take-home assignments of Weeks 2-5 and are listed in the [`Coursework` index](Coursework/README.md). The take-home assignment of Week 6 (HW5) is not part of the coursework.

---

<br>

## 6. References

| Type | Contents |
|:-----|:-----|
| Textbook | A. Silberschatz, P. B. Galvin, and G. Gagne, *Operating System Concepts*, 10th ed. Hoboken, NJ, USA: Wiley, 2018. (the Dinosaur Book) |
| Reference Course | MIT 6.1810 Operating System Engineering ([course website](https://pdos.csail.mit.edu/6.1810/)), resources for the RISC-V xv6 lab environment |
| Lecture Slides | [Instructor's Markdown notes and slides (GitHub)](https://github.com/codingchild2424/2026-lecture-operating-system) |
