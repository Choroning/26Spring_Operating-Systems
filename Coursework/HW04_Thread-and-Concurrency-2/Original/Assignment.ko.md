# HW4 과제 안내문 — 암묵적 스레딩

[레포지토리 안내](../../../README.ko.md) | [English](Assignment.md) | [한국어](Assignment.ko.md)

> **최종 수정일:** 2026-11-16

운영체제 4차 과제이다. 5주차(스레드와 동시성 2)의 암묵적 스레딩 가운데 포크-조인 병렬성과 OpenMP를 다룬다. 배포 파일인 시작 코드 `skeleton/mergesort.c`와 `skeleton/matmul.c`는 이 폴더에 원본 그대로 있다.

- **교재:** Silberschatz Ch 4 (4.5\~4.7절)
- **제출 기한:** 다음 수업 전
- **환경:** gcc를 쓸 수 있는 Linux 또는 macOS (OpenMP는 `-fopenmp` 필요)

---

<br>

## 과제 1: 포크-조인 병렬 병합 정렬 (필수)

교재 Ch 4.5.2(Fork-Join Parallelism)를 바탕으로 한다.

### 배경

**포크-조인(fork-join)** 은 작업을 하위 작업으로 나누고(fork), 각 하위 작업을 병렬로 실행한 뒤,
결과를 합치는(join) 병렬 패턴이다.
이 패턴은 병합 정렬 같은 분할 정복 알고리즘에 자연스럽게 대응한다.

```
            [8, 3, 1, 5, 4, 7, 2, 6]
                    fork
           /                    \
    [8, 3, 1, 5]          [4, 7, 2, 6]
        fork                   fork
       /    \                 /    \
   [8,3]  [1,5]          [4,7]  [2,6]
     |      |               |      |
   [3,8]  [1,5]          [4,7]  [2,6]
       \    /                 \    /
    [1, 3, 5, 8]          [2, 4, 6, 7]
           \                    /
                    join
            [1, 2, 3, 4, 5, 6, 7, 8]
```

순차 병합 정렬은 나눈 부분을 한 번에 하나씩 처리한다.
포크-조인 병합 정렬은 두 절반을 스레드로 **병렬로** 정렬한다.

### 목표

Pthreads로 다음을 만족하는 병렬 병합 정렬을 구현한다.
1. 한 절반을 정렬할 새 스레드를 **포크**하고, 현재 스레드는 나머지 절반을 정렬한다
2. 그 스레드를 **조인**하고 정렬된 두 절반을 병합한다
3. 스레드를 너무 많이 만들지 않도록 **깊이 임계값**을 둔다 (임계값 아래에서는 순차 정렬로 돌아간다)
4. 순차 버전과 성능을 비교한다

### 구현 요구사항

#### 1. `merge()`: 정렬된 두 절반의 병합

표준 병합 연산이다. 시작 코드에 이미 들어 있다.

#### 2. `merge_sort_sequential()`: 순차 병합 정렬

스레드를 쓰지 않는 표준 재귀 병합 정렬이다. 비교의 기준이 되고,
깊이 임계값에 이르렀을 때 대신 실행된다.

#### 3. `merge_sort_parallel()`: 병렬 포크-조인 병합 정렬

```c
struct sort_arg {
    int *array;
    int *temp;
    int left;
    int right;
    int depth;      /* 현재 재귀 깊이 */
};

void *merge_sort_parallel(void *arg)
{
    // 배열 크기가 1 이하이면 반환한다
    // depth >= MAX_DEPTH이면 순차 정렬로 돌아간다
    //
    // 그렇지 않으면:
    //   1. 포크: 왼쪽 절반을 맡을 스레드를 만든다
    //   2. 오른쪽 절반은 현재 스레드에서 정렬한다 (재귀 호출)
    //   3. 조인: 왼쪽 절반의 스레드를 기다린다
    //   4. 정렬된 두 절반을 병합한다
}
```

**설계의 핵심:**
- 한 단계에서 스레드를 두 개가 아니라 하나만 포크한다. 나머지 절반은 현재 스레드가 맡는다
- `MAX_DEPTH`는 스레드 수를 2^MAX_DEPTH개로 제한한다
- `MAX_DEPTH` 아래에서는 `merge_sort_sequential()`로 돌아간다

#### 4. `main()`: 성능 비교

- 정수 N개의 무작위 배열을 만든다
- 순차 병합 정렬로 정렬하고 시간을 잰다
- 병렬 병합 정렬로 정렬하고 시간을 잰다
- 속도 향상 비율을 출력한다
- 두 버전이 같은 정렬 결과를 내는지 검증한다

### 시작하기

```bash
cd skeleton/
gcc -Wall -pthread -O2 -o mergesort mergesort.c
./mergesort              # 기본값: 원소 10,000,000개, 깊이 3
./mergesort 5000000 4    # 지정: 원소 500만 개, 깊이 4
```

`mergesort.c`에서 `TODO` 주석을 찾아 코드를 완성한다.

### 예상 출력

```
=== Fork-Join Parallel Merge Sort ===
Array size: 10000000, Max thread depth: 3 (up to 8 threads)

Sequential: 1.2345 sec
Parallel:   0.4567 sec
Speedup:    2.70x
Verified:   CORRECT
```

### 생각해 볼 질문

1. `MAX_DEPTH`가 너무 크면 어떻게 되는가? (힌트: 스레드 생성 오버헤드)
2. 각 단계에서 스레드를 두 개가 아니라 하나만 포크하는 이유는 무엇인가?
3. 이것은 교재에서 다룬 Java의 `ForkJoinPool`과 어떤 관계가 있는가?
4. 속도 향상을 암달의 법칙과 비교하라. 순차 비율은 얼마인가?

### 채점 기준

| 항목 | 비중 |
|------|--------|
| 스레드 포크와 조인을 갖춘 올바른 `merge_sort_parallel()` | 30% |
| 스레드 수를 제한하는 적절한 깊이 임계값 | 20% |
| 정렬된 결과를 내는 올바른 병합 | 20% |
| 성능 비교 (순차와 병렬) | 20% |
| 코드 품질과 질문의 답 | 10% |

*표 1. 과제 1의 채점 기준*

---

<br>

## 과제 2: OpenMP 행렬 곱셈 (보너스, +20점)

교재 Ch 4.5.3(OpenMP)을 바탕으로 한다.

### 배경

행렬 곱셈은 전형적인 데이터 병렬 작업이다. 결과 행렬 C의
각 원소는 독립적으로 계산할 수 있다.

```
C[i][j] = sum(A[i][k] * B[k][j]) for k = 0..N-1
```

그래서 OpenMP의 `#pragma omp parallel for`에 알맞다.

### 목표

행렬 곱셈을 세 가지 버전으로 구현하고 성능을 비교한다.
1. **순차**: 병렬성 없음
2. **OpenMP 기본**: 가장 바깥 루프에 `#pragma omp parallel for`
3. **OpenMP 최적화**: `collapse(2)`나 루프 순서 바꾸기(i-k-j), 또는 둘 다

### 구현 요구사항

#### 1. `matmul_sequential()`: 기준 구현

표준 3중 루프(i-j-k 순서)이다.

#### 2. `matmul_openmp_basic()`: 기본 OpenMP

가장 바깥 루프에 `#pragma omp parallel for`를 더한다.

#### 3. `matmul_openmp_optimized()`: 최적화한 OpenMP

다음 최적화 중 하나 또는 둘 다를 적용한다.
- **`collapse(2)`**: i 루프와 j 루프를 함께 병렬화하여 부하를 고르게 나눈다
- **루프 순서 바꾸기 (i-k-j)**: B를 순차적으로 접근하므로 캐시 지역성이 좋아진다

### 시작하기

```bash
cd skeleton/
gcc -Wall -fopenmp -O2 -o matmul matmul.c
./matmul              # 기본값: 1024x1024
./matmul 512          # 지정: 512x512
```

### 예상 출력

```
=== OpenMP Matrix Multiplication ===
Matrix size: 1024 x 1024

Sequential (i-j-k):     3.4567 sec
OpenMP basic:            1.2345 sec  (2.80x)
OpenMP optimized (i-k-j): 0.5678 sec  (6.09x)
Verified: CORRECT
```

### 채점 기준

| 항목 | 비중 |
|------|--------|
| 올바른 순차 행렬 곱셈 | 20% |
| 올바른 OpenMP 기본 버전 | 30% |
| 측정 가능한 개선이 있는 최적화 버전 | 30% |
| 성능 비교와 분석 | 20% |

*표 2. 과제 2의 채점 기준*

---

<br>

## 제출물

```
week05/3_assignment/
├── skeleton/
│   ├── mergesort.c        <- TODO를 완성해 제출 (필수)
│   └── matmul.c           <- TODO를 완성해 제출 (보너스)
└── report.md              <- 간단한 구현 설명 (선택)
```

### report.md 작성 안내 (선택)

다음 질문에 답하면 가산점을 준다.

1. `MAX_DEPTH`는 성능에 어떤 영향을 주는가? 자신의 컴퓨터에서 가장 좋은 값은 얼마인가?
2. 루프 순서를 i-k-j로 바꾸면 캐시 성능이 좋아지는 이유는 무엇인가?
3. 포크-조인 패턴과 OpenMP를 비교하라. 각각은 언제 쓰는가?

---

<br>

## 참고 자료

- Silberschatz Ch 4.5.2: Fork-Join Parallelism
- Silberschatz Ch 4.5.3: OpenMP
- Java ForkJoinPool 문서 (개념 비교용)

---

<br>

문제를 풀어 본 뒤 [과제 해설](../Assignment-Explanation.ko.md)에서 정답과 풀이를, [코드 총정리](../Code-Walkthrough.ko.md)에서 작성한 코드 전체를 볼 수 있다.
