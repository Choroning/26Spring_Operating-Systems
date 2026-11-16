# HW3 과제 안내문 — Pthreads 데이터 병렬성

[레포지토리 안내](../../../README.ko.md) | [English](Assignment.md) | [한국어](Assignment.ko.md)

> **최종 수정일:** 2026-11-16

운영체제 3차 과제이다. 4주차(스레드와 동시성 1)의 Pthreads와 데이터 병렬성을 다룬다. 배포 파일인 시작 코드 `skeleton/histogram.c`는 이 폴더에 원본 그대로 있다.

- **교재:** Silberschatz Ch 4 (Threads & Concurrency)
- **제출 기한:** 다음 수업 전
- **환경:** gcc와 pthreads를 쓸 수 있는 Linux 또는 macOS

---

<br>

## 과제: 병렬 히스토그램 (필수)

### 배경

**히스토그램(histogram)** 은 데이터 집합에서 각 값이 몇 번 나타나는지 센다.
예를 들어 학생 1,000만 명의 0\~99점 시험 점수가 주어지면,
히스토그램은 각 점수를 받은 학생이 몇 명인지 센다.

이것은 전형적인 **데이터 병렬성(data parallelism)** 문제이다. 데이터 집합을 N개의 스레드에 나누고,
각 스레드가 자신의 몫에 대해 지역 히스토그램을 만든 뒤,
메인 스레드가 결과를 합친다.

```
Data: [72, 85, 72, 91, 85, 60, ...]   (10,000,000 values)

Thread 0: [72, 85, 72, ...]  → local_hist0[72]=2, [85]=1, ...
Thread 1: [91, 85, 60, ...]  → local_hist1[91]=1, [85]=1, [60]=1, ...
Thread 2: ...                 → local_hist2[...]
Thread 3: ...                 → local_hist3[...]

Merge: global_hist[i] = sum of local_hist*[i] for all threads
```

스레드마다 자신의 지역 히스토그램(별도의 배열)에서 작업하므로,
세는 단계에서는 동기화가 필요 없다. 이것은 실습 2의
`partial_sum[id]`와 같은 패턴이다.

### 학습 목표

- Pthreads(`pthread_create`, `pthread_join`)로 **데이터 병렬성**을 적용한다
- **스레드별 배열**을 따로 두어 공유로 인한 충돌을 피한다
- 스레드 수를 바꾸며 **속도 향상**을 측정하고 **암달의 법칙**과 연결한다
- 교재의 생성 루프와 조인 루프 패턴을 연습한다

### 구현 요구사항

#### 1. 데이터 생성

범위 `[0, NUM_BINS)`의 무작위 정수 `N`개로 이루어진 배열을 만든다.

```c
#define NUM_BINS 100        /* 히스토그램 구간 (예: 시험 점수 0-99) */
#define DEFAULT_SIZE 10000000
```

#### 2. `histogram_sequential()`: 기준 구현

```c
void histogram_sequential(int *data, int n, int *hist)
{
    memset(hist, 0, NUM_BINS * sizeof(int));
    for (int i = 0; i < n; i++)
        hist[data[i]]++;
}
```

#### 3. `histogram_parallel()`: 병렬 구현

각 스레드는
1. 자신이 맡을 범위(`start`, `end`)와 **지역 히스토그램 배열**을 받는다
2. `data[start..end)`의 값을 지역 히스토그램에 센다
3. 반환한다 (공유 상태를 바꾸지 않는다)

메인 스레드는
1. 각자의 지역 히스토그램을 가진 스레드 `nthreads`개를 만든다
2. 모든 스레드를 조인한다
3. 지역 히스토그램을 전역 결과로 합친다.
   모든 스레드 t에 대해 `global_hist[i] = sum of local_hist[t][i]`

#### 4. `main()`: 성능 비교

- 순차 버전과 병렬 버전(스레드 1, 2, 4, 8개)을 실행한다
- 각각의 경과 시간과 속도 향상을 출력한다
- 병렬 결과가 순차 결과와 같은지 검증한다

### 시작하기

```bash
cd skeleton/
gcc -Wall -pthread -O2 -o histogram histogram.c
./histogram                    # 기본값: 1,000만 개, 스레드 4개
./histogram 20000000 8         # 지정: 2,000만 개, 스레드 8개
```

`histogram.c`에서 `TODO` 주석을 찾아 코드를 완성한다.

### 예상 출력

```
=== Parallel Histogram ===
Data size: 10000000, Bins: 100

Sequential:          0.0312 sec

Parallel (1 thread): 0.0315 sec  Speedup: 0.99x
Parallel (2 threads): 0.0178 sec  Speedup: 1.75x
Parallel (4 threads): 0.0098 sec  Speedup: 3.18x
Parallel (8 threads): 0.0067 sec  Speedup: 4.66x

Verification: ALL CORRECT

--- Top 5 most frequent values ---
Value 42: 100,234 occurrences
Value 17: 100,198 occurrences
...
```

### 질문 (보고서에 포함)

1. 각 스레드가 공유 히스토그램 대신 **지역 히스토그램**을 쓰는 이유는 무엇인가?
   모든 스레드가 `global_hist[data[i]]++`를 직접 증가시키면 어떤 일이 생기는가?

2. 스레드가 1, 2, 4, 8개일 때 속도 향상이 어떻게 변하는지 그래프로 그리거나 서술하라.
   암달의 법칙의 예측과 맞는가? 순차 비율은 얼마인가?

3. 스레드를 만들고 조인하는 오버헤드는 얼마인가? 매우 작은 데이터 집합(예: 1000개)으로
   시험해 보라. 그래도 병렬 버전이 더 빠른가?

4. 이 패턴을 실습 2(병렬 배열 합)와 비교하라. 무엇이 같고,
   무엇이 다른가?

### 채점 기준

| 항목 | 비중 |
|------|--------|
| 범위를 나누어 스레드를 올바르게 생성 | 25% |
| 스레드별 지역 히스토그램 (공유 충돌 없음) | 25% |
| 지역 히스토그램의 올바른 병합 | 20% |
| 스레드 수별 성능 측정 | 20% |
| 보고서의 답 | 10% |

*표 1. 채점 기준*

---

<br>

## 제출물

```
week04/3_assignment/
├── skeleton/
│   └── histogram.c     <- TODO를 완성해 제출
└── report.md           <- 성능 결과와 질문의 답
```

---

<br>

## 참고 자료

- Silberschatz Ch 4.2: Multicore Programming, Amdahl's Law
- Silberschatz Ch 4.4: Pthreads (create, join, data sharing)
- 실습 2: 데이터 병렬 배열 합 (같은 패턴, 더 단순한 규모)

---

<br>

문제를 풀어 본 뒤 [과제 해설](../Assignment-Explanation.ko.md)에서 정답과 풀이를, [코드 총정리](../Code-Walkthrough.ko.md)에서 작성한 코드 전체를 볼 수 있다.
