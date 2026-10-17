# [2026학년도 봄학기] 운영체제

![Last Commit](https://img.shields.io/github/last-commit/Choroning/26Spring_Operating-Systems)
![Languages](https://img.shields.io/github/languages/top/Choroning/26Spring_Operating-Systems)

이 레포지토리는 운영체제 과목의 이중 언어 강의 노트와 수업 결과물을 정리한 아카이브입니다.

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

이 레포지토리에는 공룡책(Operating System Concepts)을 바탕으로 운영체제의 설계와 구현을 다루는 운영체제 과목의 강의 노트와 수업 결과물, 시스템 수준 코드가 담겨 있습니다.

- `Lecture-Notes/`에는 주차마다 이론 강의의 개념을 정리한 강의 노트와 1~6주차 실습 세션을 정리한 실습 노트가 있습니다. 강의자료가 주차별로 배포되어 단원도 주차(`Week##`)로 나눴으며, 단원들은 공룡책 기반의 전체 커리큘럼을 다룹니다. 중간고사가 있는 8주차에는 단원이 없습니다.
- 모든 강의 노트와 실습 노트는 영어 파일(`.md`)과 한국어 파일(`.ko.md`)이 같은 구성으로 짝을 이룹니다.
- `Coursework/`에는 과제 풀이(C 구현, xv6 커널 패치, 제출 보고서)와 중간고사 대비 자료가 있습니다. HW1에는 이중 언어 해설 문서(`.ko.md`, `.md`)와 풀이를 자동으로 확인하는 `.sh` 채점 스크립트가 함께 있습니다.
- 팀 단위 텀 프로젝트의 코드와 실습 환경인 xv6-riscv는 `Coursework/` 아래에 git submodule로 연결되어 있습니다.

> **🤖 AI 에이전트 활용**
> 이 과목은 AI 에이전트 사용을 권장합니다.
> 수업 전반에 걸쳐 [Claude Code](https://claude.ai/download)와 [Gemini CLI](https://github.com/google-gemini/gemini-cli)를 코딩 어시스턴트로 활용했고, 레포지토리 구조 정리에는 Claude Code를 활용했습니다.

<br><a name="course-information"></a>
## 📚 강의 정보

- **학기:** 2026학년도 봄학기 (3월 - 6월)
- **소속:** 고려대학교(세종)

|학수번호      |강의명    |이수구분|교수자|개설학과|
|:----------:|:-------|:----:|:------:|:----------------|
|`DCSS301-00`|운영체제|전공필수|이웅기 교수|컴퓨터소프트웨어학과|

### 과목 개요

운영체제의 설계와 구현을 다루며, 프로세스와 스레드, CPU 스케줄링, 동기화, 메모리 관리, 파일 시스템, 입출력 등을 학습한다. 이론적 기반과 함께 Linux, UNIX, xv6의 구현 사례를 살펴본다. 교수자와 연구실, 수업 일정과 형식, 평가, 수업 운영 규정, 주차별 계획 및 진도, 참고 자료는 [강의 정보 문서](Course-Information.ko.md)에 정리했다.

<br><a name="prerequisites"></a>
## ✅ 사전 요구사항

- 컴퓨터 구조와 C 프로그래밍에 대한 이해가 필요합니다.
- C 컴파일러(예: GCC, Clang)가 설치되어 있어야 합니다.
- Unix/Linux 셸 환경에 익숙해야 합니다.

- **💻 개발 환경**

| 도구 | 회사 |  운영체제  | 비고 |
|:-----|:-------:|:----:|:------|
|Visual Studio Code|Microsoft|macOS|    |
|Xcode|Apple Inc.|macOS|    |

<br><a name="repository-structure"></a>
## 🗂 레포지토리 구조

```plaintext
26Spring_Operating-Systems
├── Lecture-Notes               주차별 강의 노트와 실습 노트
├── Coursework                  과제, 시험 대비 자료, 텀 프로젝트, xv6 실습 환경
├── images                      모든 문서의 그림
├── Course-Information.ko.md    수업 운영 정보
├── Course-Information.md
├── LICENSE                     문서 라이선스 (CC BY-NC-SA 4.0)
├── LICENSE-CODE                코드 라이선스 (PolyForm Noncommercial 1.0.0)
├── README.ko.md
└── README.md
```

단원 목록은 [`Lecture-Notes` 색인](Lecture-Notes/README.ko.md)에, 과제와 텀 프로젝트는 [`Coursework` 색인](Coursework/README.ko.md)에 있습니다.

<br><a name="license"></a>
## 🤝 라이선스

이 레포지토리의 문서(강의 노트, 해설, 요약지)는 [CC BY-NC-SA 4.0](LICENSE), 코드는 [PolyForm Noncommercial 1.0.0](LICENSE-CODE)에 따라 배포됩니다. 강의자료와 교재에서 가져온 그림과 예제 코드, 강의가 배포한 과제 원본과 템플릿 코드는 원저작자에게 권리가 있으며 이 라이선스의 대상이 아닙니다.

---
