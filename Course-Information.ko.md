# 강의 정보

[레포지토리 안내](README.ko.md) | [English](Course-Information.md) | [한국어](Course-Information.ko.md)

> **최종 수정일:** 2026-10-18

## 목차

- [1. 교수자 및 연구실](#1-교수자-및-연구실)
- [2. 수업 일정 및 형식](#2-수업-일정-및-형식)
- [3. 평가](#3-평가)
- [4. 수업 운영 규정](#4-수업-운영-규정)
- [5. 주차별 계획 및 진도](#5-주차별-계획-및-진도)
- [6. 참고 자료](#6-참고-자료)

---

<br>

## 1. 교수자 및 연구실

- **교수자:** 이웅기 교수, 컴퓨터소프트웨어학과
- **연구실:** [LEAP Lab](https://codingchild2424.github.io/lab-website/). 교육 분야 생성형 AI, 교수학적 정렬, 대규모 언어 모델, 지식 추적을 연구한다.

---

<br>

## 2. 수업 일정 및 형식

- **학점:** 3학점
- **수업 시간:** 수요일 5~6교시, 목요일 8교시
- **강의실:** 과학기술2관 310호
- **주간 형식:** 1교시 이론 강의(전반부)와 퀴즈, 2교시 이론 강의(후반부), 3교시 실습
- **실습 환경:** RISC-V용 xv6를 사용하며 [MIT 6.1810](https://pdos.csail.mit.edu/6.1810/) 자료를 참고한다.

---

<br>

## 3. 평가

| 항목 | 비율 |
|:-----|-----:|
| 과제 (퀴즈 5%, 가정 과제 5%) | 10% |
| 중간고사 (필기) | 30% |
| 기말고사 (필기) | 30% |
| 텀 프로젝트 | 30% |
| 출석 | 0% |

- 퀴즈는 3\~7주차와 9\~13주차에 총 10회, 가정 과제는 2\~6주차에 총 5회 실시한다.
- 필기시험은 전자기기 없이 손글씨로 1시간 동안 진행한다.
- 팀 단위 텀 프로젝트는 9주차에 시작한다. 3~4명으로 팀을 구성해 운영체제 시제품을 설계하고 개발하며, 명세서와 프로젝트 보고서를 작성해 14주차에 대면 발표를 진행한다.
- 텀 프로젝트 평가는 교수자 평가와 동료 평가가 각각 절반을 차지한다.

---

<br>

## 4. 수업 운영 규정

- 과제와 텀 프로젝트에서 생성형 AI 도구를 사용할 수 있으며 사용을 권장한다. 이때 자신의 추론과 설계 결정을 설명해야 한다.
- 전체 수업 시수의 3분의 1을 초과해 결석하면 성적이 부여되지 않는다.

---

<br>

## 5. 주차별 계획 및 진도

| 주차 | 수업일 (수, 목) | 계획된 주제 | 진행한 강의자료 |
|:-----|:-----|:-----|:-----|
| 1 | 3/4, 3/5 | 운영체제 소개 | W01_LEC_Introduction<br>W01_LAB_Coding-Agents |
| 2 | 3/11, 3/12 | 프로세스 1 | W02_LEC_Process-1<br>W02_LAB_Process-System-Calls<br>HW1 |
| 3 | 3/18, 3/19 | 프로세스 2 | W03_LEC_Process-2<br>W03_LAB_Exploring-XV6-Internals<br>HW2 |
| 4 | 3/25, 3/26 | 스레드와 동시성 1 | W04_LEC_Thread-And-Concurrency-1<br>W04_LAB_Pthread-Basics<br>HW3 |
| 5 | 4/1, 4/2 | 스레드와 동시성 2 | W05_LEC_Thread-And-Concurrency-2<br>W05_LAB_Implicit-Threading-And-Threading-Issues<br>HW4 |
| 6 | 4/8, 4/9 | CPU 스케줄링 1 | W06_LEC_CPU-Scheduling-1<br>W06_LAB_Context-Switching<br>HW5 |
| 7 | 4/15, 4/16 | CPU 스케줄링 2 | W07_LEC_CPU-Scheduling-2 |
| 8 | 4/22, 4/23 | 중간고사 | 중간고사 |
| 9 | 4/29, 4/30 | 동기화 도구와 사례 | W09_LEC_Synchronization<br>W09_LAB_Sleep-Wakeup-And-Pipes<br>텀 프로젝트 시작 |
| 10 | 5/6, 5/7 | 교착 상태 | W10_LEC_Deadlocks<br>W10_LAB_Deadlocks |
| 11 | 5/13, 5/14 | 주 메모리 | W11_LEC_Main-Memory<br>W11_LAB_Page-Tables |
| 12 | 5/20, 5/21 | 가상 메모리 | W12_LEC_Virtual-Memory<br>W12_LAB_COW-Fork-And-Lazy-Allocation |
| 13 | 5/27, 5/28 | 저장 장치 관리, 보안 및 보호 | W13_LEC_Storage-Management<br>W13_LAB_File-System-Internals |
| 14 | 6/3, 6/4 | 기말고사 (텀 프로젝트) | W14_LEC_Security-And-Protection |
| 15 | 6/10, 6/11 | 기말고사 (필기) | - |
| 16 | 6/17, 6/18 | 자율학습 주간 | - |

- 강의자료는 주차마다 이론 자료(`W##_LEC_`)와 실습 자료(`W##_LAB_`)로 배포되었으며, 같은 번호의 `Week##` 단원에 각각 `Concepts_Lecture`와 `Concepts_Lab`으로 정리했다. 단원 목록은 [`Lecture-Notes` 색인](Lecture-Notes/README.ko.md)에 있다.
- 7주차와 14주차에는 실습 자료가 없고, 9~13주차의 실습 자료(`W09_LAB_Sleep-Wakeup-And-Pipes`, `W10_LAB_Deadlocks`, `W11_LAB_Page-Tables`, `W12_LAB_COW-Fork-And-Lazy-Allocation`, `W13_LAB_File-System-Internals`)는 실습 노트를 만들지 않았다.
- 보안 및 보호는 13주차에 계획된 주제이지만 강의자료가 14주차 자료(`W14_LEC_Security-And-Protection`)로 배포되어 `Week14` 단원에 정리했다.
- HW1\~HW4는 2\~5주차의 가정 과제이며 [`Coursework` 색인](Coursework/README.ko.md)에 있다. 6주차의 가정 과제(HW5)는 수업 결과물에 포함되어 있지 않다.

---

<br>

## 6. 참고 자료

| 유형 | 내용 |
|:-----|:-----|
| 교재 | A. Silberschatz, P. B. Galvin, and G. Gagne, *Operating System Concepts*, 10th ed. Hoboken, NJ, USA: Wiley, 2018. (공룡책) |
| 참고 강의 | MIT 6.1810 Operating System Engineering ([강의 웹사이트](https://pdos.csail.mit.edu/6.1810/)), 실습 환경인 RISC-V용 xv6의 자료 |
| 강의자료 | [교수자 제공 Markdown 노트와 슬라이드 (GitHub)](https://github.com/codingchild2424/2026-lecture-operating-system) |
