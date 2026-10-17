# 강의 노트

[레포지토리 안내](../README.ko.md) | [English](README.md) | [한국어](README.ko.md)

> **최종 수정일:** 2026-10-18

| 단원 | 주제 | 문서 |
|:-----|:-----|:-----|
| [1주차 운영체제 소개](Week01_Introduction-to-Operating-Systems/) | 운영체제의 정의와 두 가지 역할, 이중 모드 동작, 시스템 콜, 저장 장치 계층, OS 구조, xv6, 학기 미리보기, 코딩 에이전트 실습 | [강의 노트](Week01_Introduction-to-Operating-Systems/Concepts_Lecture.ko.md), [실습 노트](Week01_Introduction-to-Operating-Systems/Concepts_Lab.ko.md) |
| [2주차 프로세스 (1)](Week02_Process-1/) | 프로세스 개념, 프로세스 스케줄링, 프로세스 연산, fork(), exec(), wait(), pipe(), dup2()와 입출력 리디렉션 | [강의 노트](Week02_Process-1/Concepts_Lecture.ko.md), [실습 노트](Week02_Process-1/Concepts_Lab.ko.md) |
| [3주차 프로세스 (2)](Week03_Process-2/) | 프로세스 간 통신(IPC), 공유 메모리와 메시지 전달, 파이프, 클라이언트-서버 통신, xv6 소스 구조와 시스템 콜 추적, struct proc과 fork() 구현 분석 | [강의 노트](Week03_Process-2/Concepts_Lecture.ko.md), [실습 노트](Week03_Process-2/Concepts_Lab.ko.md) |
| [4주차 스레드와 동시성 (1)](Week04_Thread-and-Concurrency-1/) | 스레드의 개념과 멀티스레딩의 이점, 멀티코어 프로그래밍, 사용자 스레드와 커널 스레드, 멀티스레딩 모델, Pthreads와 Java 스레드, 데이터 병렬성과 암달의 법칙 | [강의 노트](Week04_Thread-and-Concurrency-1/Concepts_Lecture.ko.md), [실습 노트](Week04_Thread-and-Concurrency-1/Concepts_Lab.ko.md) |
| [5주차 스레드와 동시성 (2)](Week05_Thread-and-Concurrency-2/) | 암묵적 스레딩(스레드 풀, Fork-Join, OpenMP, GCD, TBB), 스레딩 이슈, 운영체제 사례, 다중 스레드 프로그램의 fork(), 스레드 로컬 저장소 | [강의 노트](Week05_Thread-and-Concurrency-2/Concepts_Lecture.ko.md), [실습 노트](Week05_Thread-and-Concurrency-2/Concepts_Lab.ko.md) |
| [6주차 CPU 스케줄링 (1)](Week06_CPU-Scheduling-1/) | 스케줄링 기준, FCFS, SJF, SRTF, 라운드 로빈, 알고리즘 비교, xv6의 문맥 교환과 스케줄러 추적 | [강의 노트](Week06_CPU-Scheduling-1/Concepts_Lecture.ko.md), [실습 노트](Week06_CPU-Scheduling-1/Concepts_Lab.ko.md) |
| [7주차 CPU 스케줄링 (2)](Week07_CPU-Scheduling-2/) | 우선순위 스케줄링, Multilevel Queue와 MLFQ, 스레드 스케줄링, 다중 프로세서 스케줄링, 실시간 CPU 스케줄링, 알고리즘 평가 | [강의 노트](Week07_CPU-Scheduling-2/Concepts_Lecture.ko.md) |
| [9주차 동기화](Week09_Synchronization/) | 임계 구역 문제, Peterson의 해법, 동기화를 위한 하드웨어 지원, 뮤텍스 락, 세마포어, 모니터, Liveness, 동기화의 고전 문제들 | [강의 노트](Week09_Synchronization/Concepts_Lecture.ko.md) |
| [10주차 데드락](Week10_Deadlocks/) | 데드락의 특성, 데드락 예방, 회피, 탐지, 복구, 은행원 알고리즘 | [강의 노트](Week10_Deadlocks/Concepts_Lecture.ko.md) |
| [11주차 주 메모리](Week11_Main-Memory/) | 주소 바인딩과 MMU, 연속 메모리 할당, 페이징과 TLB, 페이지 테이블 구조, 스와핑 | [강의 노트](Week11_Main-Memory/Concepts_Lecture.ko.md) |
| [12주차 가상 메모리](Week12_Virtual-Memory/) | Demand Paging, Copy-on-Write, 페이지 교체와 LRU 근사 알고리즘, 프레임 할당과 Thrashing, 메모리 압축, 커널 메모리 할당 | [강의 노트](Week12_Virtual-Memory/Concepts_Lecture.ko.md) |
| [13주차 저장 장치 관리](Week13_Storage-Management/) | HDD 구조와 접근 시간, NVM 장치, HDD 스케줄링 알고리즘, 오류 탐지와 정정, Swap-Space 관리, 저장 장치 부착, RAID | [강의 노트](Week13_Storage-Management/Concepts_Lecture.ko.md) |
| [14주차 보안과 보호](Week14_Security-Protection/) | 프로그램 위협, 시스템과 네트워크 위협, 암호학, 사용자 인증, 보호 링과 보호 도메인, 접근 행렬, 접근 제어 모델, 샌드박싱과 코드 서명 | [강의 노트](Week14_Security-Protection/Concepts_Lecture.ko.md) |
