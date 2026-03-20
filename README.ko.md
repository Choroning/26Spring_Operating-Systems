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
│   ├── Concepts-Lab.ko.md
│   ├── Concepts-Lab.md
│   ├── Concepts-Lecture.ko.md
│   └── Concepts-Lecture.md
├── W02_Process-1
│   ├── Assignment
│   │   ├── minishell.c
│   │   ├── pingpong.c
│   │   ├── test_minishell.sh
│   │   └── test_pingpong.sh
│   ├── Assignment-Explanation.ko.md
│   ├── Assignment-Explanation.md
│   ├── Concepts-Lab.ko.md
│   ├── Concepts-Lab.md
│   ├── Concepts-Lecture.ko.md
│   └── Concepts-Lecture.md
├── W03_Process-2
│   ├── Concepts-Lab.ko.md
│   ├── Concepts-Lab.md
│   ├── Concepts-Lecture.ko.md
│   └── Concepts-Lecture.md
├── W04_Thread-And-Concurrency-1
├── W05_Thread-And-Concurrency-2
├── W06_CPU-Scheduling-1
├── W07_CPU-Scheduling-2
├── W09_Synchronization
├── W10_Deadlocks
├── W11_Main-Memory
├── W12_Virtual-Memory
├── W13_Storage-Management
├── W14_Security-And-Protection
├── images
│   ├── *.png                          (lecture diagrams and icons)
│   ├── cropped
│   │   └── (cropped figure images)
│   └── figures
│       └── (extracted lecture figures)
├── LICENSE
├── README.ko.md
└── README.md
```

<br><a name="license"></a>
## 🤝 라이선스

이 레포지토리는 [MIT License](LICENSE) 하에 배포됩니다.

---
