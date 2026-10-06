# [2026학년도 봄학기] 운영체제

![Last Commit](https://img.shields.io/github/last-commit/Choroning/26Spring_Operating-Systems)
![Languages](https://img.shields.io/github/languages/top/Choroning/26Spring_Operating-Systems)

이 레포지토리는 대학 강의 및 과제를 위해 작성된 C 및 셸 스크립트 예제 코드를 체계적으로 정리하고 보관합니다.

*작성자: 박철원 (고려대학교(세종), 컴퓨터소프트웨어학과) - 2026년 기준 3학년*
<br><br>

## 📑 목차

- [레포지토리 소개](#about-this-repository)
- [강의 정보](#course-information)
- [사전 요구사항](#prerequisites)
- [레포지토리 구조](#repository-structure)
- [라이선스](#license)

---


<br><a name="about-this-repository"></a>
## 📝 레포지토리 소개

이 레포지토리에는 대학 수준의 운영체제 과목을 위해 작성된 이중 언어 학습 자료와 시스템 수준 코드가 포함되어 있습니다:

- 매 강의 및 실습 세션별 이중 언어 개념 정리 노트 (한국어 `.ko.md` + 영어 `.md`)
- 이중 언어 설명 문서(`.ko.md` + `.md`)를 포함한 과제 솔루션
- C 구현 및 `.sh` 채점 스크립트
- 공룡책 기반 전체 커리큘럼을 다루는 주차별 디렉토리 구조

> **🤖 AI 에이전트 활용**
> 본 과목은 AI 에이전트 사용을 권장합니다.
> 수업 전반에 걸쳐 [Claude Code](https://claude.ai/download)와 [Gemini CLI](https://github.com/google-gemini/gemini-cli)를 코딩 어시스턴트로 활용하였습니다.

<br><a name="course-information"></a>
## 📚 강의 정보

- **학기:** 2026학년도 봄학기 (3월 - 6월)
- **소속:** 고려대학교(세종)

|학수번호      |강의명    |이수구분|교수자|개설학과|
|:----------:|:-------|:----:|:------:|:----------------|
|`DCSS301-00`|운영체제|전공필수|이웅기 교수|컴퓨터소프트웨어학과|

### 과목 개요

운영체제의 설계와 구현을 다루며, 프로세스와 스레드, CPU 스케줄링, 동기화, 메모리 관리, 파일 시스템, 입출력 등을 학습한다. 이론적 기반과 함께 Linux, UNIX, xv6의 구현 사례를 살펴본다.

### 교수자 및 실습

- **교수자:** 이웅기 교수, 컴퓨터소프트웨어학과
- **연구실:** [LEAP Lab](https://codingchild2424.github.io/lab-website/) — 교육 분야 생성형 AI, 교수학적 정렬, 대규모 언어 모델, 지식 추적 연구
- **실습:** RISC-V용 xv6를 사용하며 [MIT 6.1810](https://pdos.csail.mit.edu/6.1810/) 자료를 참고한다.

### 수업 일정 및 형식

- **학점:** 3학점
- **수업 시간:** 수요일 5–6교시, 목요일 8교시
- **강의실:** 과학기술2관 310호
- **주간 형식:** 1교시 이론 강의(전반부) 및 퀴즈, 2교시 이론 강의(후반부), 3교시 실습

### 평가

| 항목 | 비율 |
|:-----|-----:|
| 과제 (퀴즈 5%, 가정 과제 5%) | 10% |
| 중간고사 (필기) | 30% |
| 기말고사 (필기) | 30% |
| 기말 프로젝트 | 30% |
| 출석 | 0% |

- 퀴즈는 3–7주차와 9–13주차에 총 10회, 가정 과제는 2–6주차에 총 5회 실시한다.
- 필기시험은 전자기기 없이 손글씨로 1시간 동안 진행한다.
- 기말 프로젝트는 9주차에 시작한다. 3–4명으로 팀을 구성해 운영체제 시제품을 설계·개발하고, 명세서와 프로젝트 보고서를 작성해 14주차 대면 발표를 진행한다. 프로젝트 평가는 교수자와 동료 평가가 각각 절반을 차지한다.
- 과제와 프로젝트에서 생성형 AI 도구를 사용할 수 있으며, 자신의 추론과 설계 결정을 설명해야 한다.
- 전체 수업 시수의 3분의 1을 초과해 결석하면 성적이 부여되지 않는다.

### 주차별 계획

| 주차 | 주제 | 주차 | 주제 |
|:----:|:-----|:----:|:-----|
| 1 | 운영체제 소개 | 9 | 동기화 도구와 사례 |
| 2 | 프로세스 1 | 10 | 교착 상태 |
| 3 | 프로세스 2 | 11 | 주 메모리 |
| 4 | 스레드와 동시성 1 | 12 | 가상 메모리 |
| 5 | 스레드와 동시성 2 | 13 | 저장 장치 관리, 보안 및 보호 |
| 6 | CPU 스케줄링 1 | 14 | 기말고사 (프로젝트) |
| 7 | CPU 스케줄링 2 | 15 | 기말고사 (필기) |
| 8 | 중간고사 | 16 | 자율학습 주간 |

- **📖 참고 자료**

| 유형 | 내용 |
|:----:|:---------|
|교재|Operating System Concepts 10판(공룡책)|
|강의자료|[교수자 제공 Markdown 노트 및 슬라이드 (GitHub)](https://github.com/codingchild2424/2026-lecture-operating-system)|

<br><a name="prerequisites"></a>
## ✅ 사전 요구사항

- 컴퓨터 구조 및 C 프로그래밍에 대한 이해
- C 컴파일러 설치(예: GCC, Clang)
- Unix/Linux 셸 환경에 익숙함

- **💻 개발 환경**

| 도구 | 회사 |  운영체제  | 비고 |
|:-----|:-------:|:----:|:------|
|Visual Studio Code|Microsoft|macOS|    |
|Xcode|Apple Inc.|macOS|    |

<br><a name="repository-structure"></a>
## 🗂 레포지토리 구조

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
## 🤝 라이선스

이 레포지토리는 [MIT License](LICENSE) 하에 배포됩니다.

---
