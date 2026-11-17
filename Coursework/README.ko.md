# 수업 결과물

[레포지토리 안내](../README.ko.md) | [English](README.md) | [한국어](README.ko.md)

> **최종 수정일:** 2026-11-18

| 항목 | 내용 | 문서 |
|:-----|:-----|:-----|
| [Exam_Midterm](Exam_Midterm/) | 1\~7주차 범위(운영체제 소개, 프로세스, IPC, 스레드와 동시성, CPU 스케줄링)의 중간고사 요약지와 총정리 | [요약지](Exam_Midterm/Summary-Sheet.ko.md), [총정리](Exam_Midterm/Total-Review.ko.md) |
| [HW01_Process-1](HW01_Process-1/) | 2주차 가정 과제. 두 프로세스가 파이프로 1바이트를 주고받고 왕복 시간을 재는 `pingpong.c`와, 단일 명령어, 입출력 리디렉션, 파이프를 지원하는 간단한 셸 `minishell.c`를 완성하는 문제로, 2주차(프로세스 1)와 이어집니다 | [과제 안내문](HW01_Process-1/Original/Assignment.ko.md), [과제 해설](HW01_Process-1/Assignment-Explanation.ko.md), [코드 총정리](HW01_Process-1/Code-Walkthrough.ko.md) |
| [HW02_Process-2](HW02_Process-2/) | 3주차 가정 과제. xv6에 호출한 프로세스의 시스템 콜을 추적하는 `trace` 시스템 콜을, 보너스로 빈 메모리의 양과 사용 중인 프로세스 수를 돌려주는 `sysinfo` 시스템 콜을 더하는 문제로, 3주차(프로세스 2)와 이어집니다 | [과제 안내문](HW02_Process-2/Original/Assignment.ko.md), [과제 해설](HW02_Process-2/Assignment-Explanation.ko.md), [코드 총정리](HW02_Process-2/Code-Walkthrough.ko.md) |
| [HW03_Thread-and-Concurrency-1](HW03_Thread-and-Concurrency-1/) | 4주차 가정 과제. 값 1,000만 개의 히스토그램을 Pthreads로 병렬화하고 스레드 수에 따른 속도 향상을 측정한 뒤, 스레드별 히스토그램, 암달의 법칙, 스레드 생성 오버헤드, 4주차 실습 2와의 비교에 관한 질문에 답하는 문제로, 4주차(스레드와 동시성 1)와 이어집니다 | [과제 안내문](HW03_Thread-and-Concurrency-1/Original/Assignment.ko.md), [과제 해설](HW03_Thread-and-Concurrency-1/Assignment-Explanation.ko.md), [코드 총정리](HW03_Thread-and-Concurrency-1/Code-Walkthrough.ko.md) |
| [HW04_Thread-and-Concurrency-2](HW04_Thread-and-Concurrency-2/) | 5주차 가정 과제. Pthreads로 포크-조인 병렬 병합 정렬을, 보너스로 OpenMP 행렬 곱셈을 구현하여 순차 버전과 성능을 비교하는 문제로, 5주차(스레드와 동시성 2)의 암묵적 스레딩과 이어집니다 | [과제 안내문](HW04_Thread-and-Concurrency-2/Original/Assignment.ko.md), [과제 해설](HW04_Thread-and-Concurrency-2/Assignment-Explanation.ko.md), [코드 총정리](HW04_Thread-and-Concurrency-2/Code-Walkthrough.ko.md) |
| [HW05_CPU-Scheduling-1](HW05_CPU-Scheduling-1/) | 6주차 가정 과제. xv6에 `setpriority`와 `getpriority` 시스템 콜을 더하고, 우선순위(0\~20, 값이 작을수록 높음, 기본값 10)가 가장 높은 프로세스를 고르며 같은 우선순위끼리는 라운드 로빈으로 실행하는 스케줄러로 바꾸는 문제로, 6주차(CPU 스케줄링 1)와 이어집니다 | [과제 안내문](HW05_CPU-Scheduling-1/Original/Assignment.ko.md), [과제 해설](HW05_CPU-Scheduling-1/Assignment-Explanation.ko.md), [코드 총정리](HW05_CPU-Scheduling-1/Code-Walkthrough.ko.md) |
| [Project_LLM-Operating-System](Project_LLM-Operating-System/) | 팀 Garbage Collector가 팀 단위 텀 프로젝트로 개발한 LLM 애플리케이션용 웹 기반 운영체제. 현재 버전 `GarbageCollector_GCOS`와 이전 버전 `GarbageCollector_OS`를 git submodule로 연결했습니다 | [GarbageCollector_GCOS](https://github.com/Choroning/GarbageCollector_GCOS), [GarbageCollector_OS](https://github.com/Choroning/GarbageCollector_OS) |
| xv6-riscv | 실습과 과제 전반에서 쓴 MIT의 교육용 운영체제 xv6의 RISC-V 버전. git submodule로 연결했습니다 | [mit-pdos/xv6-riscv](https://github.com/mit-pdos/xv6-riscv) |
