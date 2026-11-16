# HW4 코드 총정리 — 암묵적 스레딩

[레포지토리 안내](../../README.ko.md) | [English](Code-Walkthrough.md) | [한국어](Code-Walkthrough.ko.md)

> **최종 수정일:** 2026-11-16

> **학습 목표:**
> 1. `mergesort.c`와 `matmul.c`의 함수 구성을 설명하고 시작 코드에서 주어진 부분과 작성한 부분을 구분할 수 있다
> 2. `merge()`가 임시 배열을 써서 정렬된 두 부분 배열을 합치는 과정을 추적할 수 있다
> 3. 여러 스레드가 같은 배열을 공유해도 병렬 병합 정렬이 올바른 이유를 설명할 수 있다
> 4. 행렬을 1차원 배열로 저장하는 방식과 세 행렬 곱셈 함수의 메모리 접근 순서를 설명할 수 있다

---

## 목차

- [1. 파일 구성](#1-파일-구성)
- [2. mergesort.c](#2-mergesortc)
  - [2.1 헤더, 상수, 인자 구조체](#21-헤더-상수-인자-구조체)
  - [2.2 merge](#22-merge)
  - [2.3 merge_sort_sequential](#23-merge_sort_sequential)
  - [2.4 merge_sort_parallel](#24-merge_sort_parallel)
  - [2.5 get_time과 is_sorted](#25-get_time과-is_sorted)
  - [2.6 main](#26-main)
- [3. matmul.c](#3-matmulc)
  - [3.1 헤더, 전역 행렬, IDX](#31-헤더-전역-행렬-idx)
  - [3.2 초기화 함수](#32-초기화-함수)
  - [3.3 세 가지 곱셈 함수](#33-세-가지-곱셈-함수)
  - [3.4 verify와 main](#34-verify와-main)
- [4. merge의 추적](#4-merge의-추적)
- [요약](#요약)
- [점검 문제](#점검-문제)

---

<br>

## 1. 파일 구성

> **풀어 보기:** 이 문서는 작성한 코드 전체와 줄별 설명을 담고 있다. 먼저 [과제 안내문](Original/Assignment.ko.md)의 문제를 직접 풀어 본 뒤 읽는다.

짝이 되는 문서 [HW4 과제 해설](Assignment-Explanation.ko.md)은 요구사항, 배경, 구현 방법, 실험 결과와 논의를 정리한다. 이 문서는 `Solution/`의 `mergesort.c`(179줄)와 `matmul.c`(145줄)를 처음부터 끝까지 읽는다. 두 파일은 배포된 `Original/skeleton/`의 TODO 주석을 코드로 바꾼 것이며, 나머지 줄은 시작 코드와 같다.

| 파일 | 줄 | 부분 | 시작 코드와의 관계 |
|:-----|:-----|:-----|:-----|
| `mergesort.c` | 1\~65 | 머리 주석, 헤더, 상수, `struct sort_arg`, `merge()`, `merge_sort_sequential()` | 그대로 |
| `mergesort.c` | 67\~110 | 전역 `max_depth`, `merge_sort_parallel()` | 본문 86\~107번 줄 작성 |
| `mergesort.c` | 112\~179 | `get_time()`, `is_sorted()`, `main()` | 그대로 |
| `matmul.c` | 1\~50 | 머리 주석, 헤더, 전역 행렬, `IDX`, 초기화 함수, `matmul_sequential()` | 그대로 |
| `matmul.c` | 52\~62 | `matmul_openmp_basic()` | 본문 작성 |
| `matmul.c` | 64\~78 | `matmul_openmp_optimized()` | 본문 작성 |
| `matmul.c` | 80\~145 | `verify()`, `main()` | 그대로 |

*표 1. 두 소스 파일의 구성과 시작 코드와의 관계*

---

<br>

## 2. mergesort.c

### 2.1 헤더, 상수, 인자 구조체

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define DEFAULT_SIZE  10000000
#define DEFAULT_DEPTH 3

/* Argument structure for parallel merge sort threads */
struct sort_arg {
    int *array;
    int *temp;
    int left;
    int right;
    int depth;
};
```

`DEFAULT_SIZE`는 기본 배열 크기 1,000만, `DEFAULT_DEPTH`는 기본 깊이 임계값 3이다. `struct sort_arg`는 스레드 함수에 넘길 인자로, 정렬할 배열과 임시 배열의 포인터, 부분 범위의 양 끝(`left`, `right`, 둘 다 포함), 현재 재귀 깊이를 담는다. 모든 스레드는 같은 `array`와 `temp`를 가리키지만 서로 겹치지 않는 범위만 다룬다.

### 2.2 merge

```c
/*
 * merge: Merge two sorted subarrays array[left..mid] and array[mid+1..right].
 * Uses temp[] as scratch space.
 */
void merge(int *array, int *temp, int left, int mid, int right)
{
    memcpy(temp + left, array + left, (right - left + 1) * sizeof(int));

    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (temp[i] <= temp[j])
            array[k++] = temp[i++];
        else
            array[k++] = temp[j++];
    }
    while (i <= mid)
        array[k++] = temp[i++];
    while (j <= right)
        array[k++] = temp[j++];
}
```

38번 줄은 병합할 구간 `array[left..right]`를 `temp`의 같은 위치로 복사한다. 그다음 `i`는 왼쪽 부분 `temp[left..mid]`, `j`는 오른쪽 부분 `temp[mid+1..right]`, `k`는 결과를 쓸 `array`의 위치를 가리킨다. 41\~46번 줄은 두 부분의 앞 원소 중 작은 것을 `array`에 쓰고, 같으면 왼쪽 것을 먼저 쓰므로(`<=`) 같은 값의 원래 순서가 유지되는 안정 정렬이 된다. 47\~50번 줄은 남은 쪽을 그대로 옮긴다. `temp`의 같은 위치만 쓰므로, 겹치지 않는 범위를 병합하는 스레드들은 `temp`를 공유해도 충돌하지 않는다.

### 2.3 merge_sort_sequential

```c
/*
 * merge_sort_sequential: Standard recursive merge sort (no threads).
 */
void merge_sort_sequential(int *array, int *temp, int left, int right)
{
    if (left >= right)
        return;

    int mid = left + (right - left) / 2;
    merge_sort_sequential(array, temp, left, mid);
    merge_sort_sequential(array, temp, mid + 1, right);
    merge(array, temp, left, mid, right);
}
```

원소가 하나 이하이면(`left >= right`) 이미 정렬된 것이므로 반환한다. 61번 줄의 `mid = left + (right - left) / 2`는 `(left + right) / 2`와 같은 값이지만, `left + right`가 `int`의 범위를 넘는 일을 막는다. 두 절반을 재귀로 정렬하고 병합한다. 시간 복잡도는 $O(n \log n)$이다.

### 2.4 merge_sort_parallel

```c
/*
 * merge_sort_parallel: Fork-join parallel merge sort.
 *
 * If depth >= max_depth, falls back to sequential.
 * Otherwise, forks a thread for the left half,
 * sorts the right half in the current thread, then joins.
 */
int max_depth;

void *merge_sort_parallel(void *arg)
{
    struct sort_arg *sa = (struct sort_arg *)arg;
    int left  = sa->left;
    int right = sa->right;
    int depth = sa->depth;

    if (left >= right)
        return NULL;

    /* Fall back to sequential when depth threshold is reached */
    if (depth >= max_depth) {
        merge_sort_sequential(sa->array, sa->temp, left, right);
        return NULL;
    }

    int mid = left + (right - left) / 2;

    /* FORK: create a thread for the left half */
    struct sort_arg left_arg = { sa->array, sa->temp, left, mid, depth + 1 };
    pthread_t tid;
    pthread_create(&tid, NULL, merge_sort_parallel, &left_arg);

    /* Sort the right half in the current thread */
    struct sort_arg right_arg = { sa->array, sa->temp, mid + 1, right, depth + 1 };
    merge_sort_parallel(&right_arg);

    /* JOIN: wait for the left-half thread */
    pthread_join(tid, NULL);

    /* MERGE: combine the two sorted halves */
    merge(sa->array, sa->temp, left, mid, right);

    return NULL;
}
```

74번 줄의 전역 변수 `max_depth`는 `main()`이 명령행 인자로 정하며, 모든 스레드가 읽기만 한다. 78\~81번 줄은 인자 구조체의 값을 지역 변수로 옮긴다. 86\~107번 줄이 작성한 부분으로, 시작 코드의 TODO 주석이 적은 다섯 단계를 그대로 따른다. 깊이가 임계값에 이르면 순차 정렬로 돌아가고(87\~90번 줄), 그렇지 않으면 가운데를 구한 뒤 왼쪽 절반의 인자 `left_arg`를 만들어 새 스레드에 넘긴다(95\~97번 줄). 오른쪽 절반은 `right_arg`로 자기 자신을 직접 호출하여 정렬하고(100\~101번 줄), `pthread_join`으로 왼쪽 스레드를 기다린 뒤(104번 줄) 병합한다(107번 줄).

이 함수가 올바른 이유는 세 가지이다. 첫째, 왼쪽 스레드와 현재 스레드는 `[left, mid]`와 `[mid+1, right]`로 겹치지 않는 범위만 쓴다. 둘째, 병합은 `pthread_join`이 반환한 뒤에 하므로, 왼쪽 스레드가 쓴 결과를 현재 스레드가 확실히 본다. 셋째, `left_arg`는 스택의 지역 변수이지만 이 함수가 조인한 뒤에야 반환하므로 새 스레드가 쓰는 동안 사라지지 않는다. `pthread_create`와 `pthread_join`의 반환값은 검사하지 않는다. 스레드 생성이 실패하면 왼쪽 절반은 정렬되지 않은 채로 남고, 유효하지 않은 `tid`로 조인하게 되므로, 견고한 구현은 생성에 실패했을 때 왼쪽 절반을 현재 스레드에서 정렬한다.

### 2.5 get_time과 is_sorted

```c
/* Helper: get wall-clock time in seconds */
double get_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* Helper: check if array is sorted */
int is_sorted(int *array, int n)
{
    for (int i = 1; i < n; i++)
        if (array[i] < array[i - 1])
            return 0;
    return 1;
}
```

`get_time()`은 단조 시계로 잰 현재 시각을 초 단위로 돌려준다. `is_sorted()`는 이웃한 두 원소를 비교하여 배열이 오름차순인지 확인하는 보조 함수이지만, `main()`은 이 함수를 부르지 않고 순차 결과와 `memcmp()`로 비교하여 검증한다. 순차 결과와 같다는 검사는 정렬되었다는 사실과 원소의 구성이 같다는 사실을 함께 확인하므로 더 강한 검사이다.

### 2.6 main

```c
int main(int argc, char *argv[])
{
    int n = (argc > 1) ? atoi(argv[1]) : DEFAULT_SIZE;
    max_depth = (argc > 2) ? atoi(argv[2]) : DEFAULT_DEPTH;

    printf("=== Fork-Join Parallel Merge Sort ===\n");
    printf("Array size: %d, Max thread depth: %d (up to %d threads)\n\n",
           n, max_depth, 1 << max_depth);

    /* Allocate arrays */
    int *original = malloc(n * sizeof(int));
    int *array    = malloc(n * sizeof(int));
    int *temp     = malloc(n * sizeof(int));

    /* Generate random data */
    srand(42);
    for (int i = 0; i < n; i++)
        original[i] = rand() % (n * 10);

    /* --- Sequential --- */
    memcpy(array, original, n * sizeof(int));
    double t0 = get_time();
    merge_sort_sequential(array, temp, 0, n - 1);
    double t1 = get_time();
    double seq_time = t1 - t0;
    printf("Sequential: %.4f sec\n", seq_time);

    /* Save sorted result for verification */
    int *expected = malloc(n * sizeof(int));
    memcpy(expected, array, n * sizeof(int));

    /* --- Parallel --- */
    memcpy(array, original, n * sizeof(int));
    struct sort_arg sa = { array, temp, 0, n - 1, 0 };
    t0 = get_time();
    merge_sort_parallel(&sa);
    t1 = get_time();
    double par_time = t1 - t0;
    printf("Parallel:   %.4f sec\n", par_time);
    printf("Speedup:    %.2fx\n", seq_time / par_time);

    /* Verify */
    int correct = (memcmp(array, expected, n * sizeof(int)) == 0);
    printf("Verified:   %s\n", correct ? "CORRECT" : "MISMATCH!");

    free(original);
    free(array);
    free(temp);
    free(expected);
    return correct ? 0 : 1;
}
```

131\~132번 줄은 배열 크기와 깊이 임계값을 명령행 인자에서 읽고, 136번 줄은 최대 스레드 수를 `1 << max_depth`로 출력한다. 139\~141번 줄은 원본, 작업용, 임시 배열을 할당하고, 144\~146번 줄은 고정된 씨앗 42로 `[0, 10n)`의 무작위 값을 만든다. 순차 정렬(149\~154번 줄)과 병렬 정렬(161\~168번 줄)은 모두 같은 원본을 복사한 배열에서 시작하며, 순차 결과는 `expected`에 보관한다. 162번 줄의 초기 인자는 전체 범위 `[0, n - 1]`과 깊이 0이고, 164번 줄은 main 스레드가 직접 `merge_sort_parallel()`을 호출한다. 171번 줄이 두 결과를 비교하여 `CORRECT` 또는 `MISMATCH!`를 출력하고, 같으면 0을 종료 코드로 돌려준다.

---

<br>

## 3. matmul.c

### 3.1 헤더, 전역 행렬, IDX

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define DEFAULT_N 1024

/* Matrices stored as flat arrays for cache friendliness */
double *A, *B, *C;
int N;

#define IDX(i, j) ((i) * N + (j))
```

`<omp.h>`는 `omp_get_max_threads()`와 `omp_get_wtime()`을 선언한다. 세 행렬은 2차원 배열이 아니라 원소 $N^2$개의 1차원 배열로 저장하고, `IDX(i, j)`가 i행 j열의 위치 `i * N + j`를 계산한다. 같은 행의 원소가 메모리에 이어서 놓이는 행 우선 저장이므로, j가 1 늘면 주소는 8바이트, i가 1 늘면 `8N`바이트 늘어난다.

### 3.2 초기화 함수

```c
/* Initialize matrices: A and B with random values, C with zeros */
void init_matrices(void)
{
    srand(42);
    for (int i = 0; i < N * N; i++) {
        A[i] = (double)(rand() % 100) / 10.0;
        B[i] = (double)(rand() % 100) / 10.0;
    }
}

void zero_C(void)
{
    memset(C, 0, N * N * sizeof(double));
}
```

`init_matrices()`는 씨앗 42로 A와 B를 0.0\~9.9의 0.1 단위 값으로 채운다. `zero_C()`는 버전마다 결과를 새로 누적할 수 있도록 C를 0으로 비운다. 세 곱셈 함수는 모두 `C[...] +=`로 누적하므로, 실행 전에 C를 비우지 않으면 앞 버전의 결과에 더해진다.

### 3.3 세 가지 곱셈 함수

```c
/*
 * matmul_sequential: Standard i-j-k triple loop.
 */
void matmul_sequential(void)
{
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                C[IDX(i, j)] += A[IDX(i, k)] * B[IDX(k, j)];
}

/*
 * matmul_openmp_basic: Add #pragma omp parallel for to outer loop.
 */
void matmul_openmp_basic(void)
{
    #pragma omp parallel for
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                C[IDX(i, j)] += A[IDX(i, k)] * B[IDX(k, j)];
}

/*
 * matmul_openmp_optimized: Loop reordering (i-k-j) + OpenMP.
 *
 * Why i-k-j? In the innermost loop (j), both C[i][j] and B[k][j]
 * are accessed with stride 1 (sequential memory access).
 * In i-j-k order, B[k][j] has stride N (cache-unfriendly).
 */
void matmul_openmp_optimized(void)
{
    #pragma omp parallel for
    for (int i = 0; i < N; i++)
        for (int k = 0; k < N; k++)
            for (int j = 0; j < N; j++)
                C[IDX(i, j)] += A[IDX(i, k)] * B[IDX(k, j)];
}
```

순차 버전(44\~50번 줄)은 시작 코드에 주어진 i-j-k 루프이다. 가장 안쪽 k 루프에서 `A[IDX(i, k)]`는 보폭 1이지만 `B[IDX(k, j)]`는 보폭 N이다.

기본 OpenMP 버전(55\~62번 줄)은 같은 루프 위에 `#pragma omp parallel for`를 붙인다. 이 지시어는 바로 다음 `for` 루프, 곧 i 루프의 반복을 스레드들에 나누어 준다. 루프 변수 `j`와 `k`는 `for` 안에서 선언되었으므로 스레드마다 따로 생기고, 각 스레드는 서로 다른 행 i의 `C` 원소만 쓰므로 경쟁이 없다. 반복을 나누는 방식은 지정하지 않았으므로 런타임의 기본 스케줄을 따르며, 모든 행의 작업량이 같아 정적으로 나누어도 부하가 고르다.

최적화 버전(71\~78번 줄)은 k 루프와 j 루프의 순서를 바꾼다. 가장 안쪽 j 루프에서 `A[IDX(i, k)]`는 변하지 않고, `B[IDX(k, j)]`와 `C[IDX(i, j)]`는 보폭 1로 이어진다. 각 `C[i][j]`에 더하는 항의 순서는 i-j-k와 같이 k = 0, 1, ...이므로, 계산하는 값은 순차 버전과 같다. 머리 주석(7번 줄)은 "loop reordering i-k-j + collapse"라고 적지만, 함수는 `collapse(2)`를 쓰지 않고 루프 순서만 바꾼다.

### 3.4 verify와 main

```c
/* Verify two results match (within floating-point tolerance) */
int verify(double *ref, double *test, int size)
{
    for (int i = 0; i < size; i++) {
        double diff = ref[i] - test[i];
        if (diff > 1e-6 || diff < -1e-6)
            return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    N = (argc > 1) ? atoi(argv[1]) : DEFAULT_N;

    printf("=== OpenMP Matrix Multiplication ===\n");
    printf("Matrix size: %d x %d\n", N, N);
    printf("Threads: %d\n\n", omp_get_max_threads());

    A = malloc(N * N * sizeof(double));
    B = malloc(N * N * sizeof(double));
    C = malloc(N * N * sizeof(double));
    double *C_ref = malloc(N * N * sizeof(double));

    init_matrices();
    double t0, t1;

    /* Sequential */
    zero_C();
    t0 = omp_get_wtime();
    matmul_sequential();
    t1 = omp_get_wtime();
    double seq_time = t1 - t0;
    printf("Sequential (i-j-k):       %.4f sec\n", seq_time);
    memcpy(C_ref, C, N * N * sizeof(double));

    /* OpenMP basic */
    zero_C();
    t0 = omp_get_wtime();
    matmul_openmp_basic();
    t1 = omp_get_wtime();
    double omp_basic_time = t1 - t0;
    int ok1 = verify(C_ref, C, N * N);
    printf("OpenMP basic:             %.4f sec  (%.2fx) %s\n",
           omp_basic_time, seq_time / omp_basic_time,
           ok1 ? "" : "MISMATCH!");

    /* OpenMP optimized */
    zero_C();
    t0 = omp_get_wtime();
    matmul_openmp_optimized();
    t1 = omp_get_wtime();
    double omp_opt_time = t1 - t0;
    int ok2 = verify(C_ref, C, N * N);
    printf("OpenMP optimized (i-k-j): %.4f sec  (%.2fx) %s\n",
           omp_opt_time, seq_time / omp_opt_time,
           ok2 ? "" : "MISMATCH!");

    printf("\nVerified: %s\n", (ok1 && ok2) ? "CORRECT" : "ERRORS FOUND");

    free(A);
    free(B);
    free(C);
    free(C_ref);
    return (ok1 && ok2) ? 0 : 1;
}
```

`verify()`는 두 결과를 원소마다 비교하여 차이가 $10^{-6}$보다 크면 실패로 본다. 덧셈 순서가 같아도 컴파일러가 곱셈과 덧셈을 FMA 하나로 합치거나 벡터화하면 반올림이 조금 달라질 수 있으므로, 정확한 일치 대신 허용 오차를 둔다. `main()`은 행렬 크기를 인자에서 읽고, `omp_get_max_threads()`로 OpenMP가 쓸 스레드 수를 출력한다. 세 버전은 매번 `zero_C()`로 C를 비운 뒤 `omp_get_wtime()`으로 시간을 재고, 순차 결과를 `C_ref`에 복사해 두어 두 OpenMP 버전을 검증한다. 출력의 괄호 안 숫자는 순차 시간에 대한 속도 향상이다.

---

<br>

## 4. merge의 추적

과제 안내문의 예에서 `[3, 8]`과 `[1, 5]`를 합치는 마지막 단계 직전을 생각한다. `array[0..3] = {3, 8, 1, 5}`, `left = 0`, `mid = 1`, `right = 3`에서 38번 줄이 이 네 원소를 `temp[0..3]`에 복사한 뒤의 진행은 다음과 같다.

| 단계 | i, j | 비교 | 쓰는 값 | array[0..3] |
|:-----|:-----|:-----|:-----|:-----|
| 1 | 0, 2 | `temp[0]=3 <= temp[2]=1` 거짓 | 1 (`j++`) | 1, 8, 1, 5 |
| 2 | 0, 3 | `3 <= temp[3]=5` 참 | 3 (`i++`) | 1, 3, 1, 5 |
| 3 | 1, 3 | `temp[1]=8 <= 5` 거짓 | 5 (`j++`) | 1, 3, 5, 5 |
| 4 | 1, 4 | `j > right`라 첫 루프 종료, 남은 왼쪽 복사 | 8 (`i++`) | 1, 3, 5, 8 |

*표 2. merge로 [3, 8]과 [1, 5]를 합치는 과정*

`array`에 결과를 쓰는 동안 비교는 `temp`에서 하므로, 덮어쓴 원소(1단계 뒤의 `array[2]` 등)가 아직 비교에 남아 있어도 결과가 틀어지지 않는다.

---

<br>

## 요약

| 개념 | 핵심 요약 |
|:-----|:-----|
| 작성한 부분 | `merge_sort_parallel()`의 본문(86\~107번 줄)과 두 OpenMP 행렬 곱셈 함수의 본문만 작성했고, 나머지는 시작 코드와 같다. |
| merge | 구간을 `temp`에 복사한 뒤 두 부분의 앞 원소를 비교하며 `array`에 쓰고, `<=` 비교로 안정 정렬이 된다. |
| 병렬 정렬의 정확성 | 스레드들이 겹치지 않는 범위만 쓰고, 병합은 조인 뒤에 하며, `left_arg`는 조인할 때까지 살아 있다. |
| 검증 | 병합 정렬은 순차 결과와 `memcmp()`로, 행렬 곱셈은 순차 결과와 허용 오차 $10^{-6}$으로 비교하며, `is_sorted()`는 쓰이지 않는다. |
| 행렬 저장 | 행렬은 행 우선 1차원 배열이고 `IDX(i, j) = i * N + j`이므로, 열 첨자가 1 늘면 8바이트, 행 첨자가 1 늘면 `8N`바이트 이동한다. |
| OpenMP 함수 | 기본 버전은 i 루프를 행 단위로 나누고, 최적화 버전은 같은 지시어에 i-k-j 순서를 써서 B와 C를 보폭 1로 읽으며 `collapse(2)`는 쓰지 않는다. |

---

<br>

## 점검 문제

1. **left_arg의 수명:** `merge_sort_parallel()`이 `pthread_join`을 호출하기 전에 반환한다면 `left_arg`에 어떤 문제가 생기는지 쓰라.

   <details>
   <summary>정답</summary>

   `left_arg`는 함수의 스택에 있는 지역 변수이므로 함수가 반환하면 그 메모리를 다른 호출이 다시 쓸 수 있다. 새 스레드가 아직 `sa->left` 같은 필드를 읽기 전이라면 엉뚱한 범위를 정렬하게 된다. 또 조인하지 않으면 병합 시점에 왼쪽 절반이 정렬되었다는 보장도 없다.

   </details>

2. **temp의 공유:** 모든 스레드가 같은 `temp` 배열을 쓰는데도 충돌하지 않는 이유를 쓰라.

   <details>
   <summary>정답</summary>

   `merge()`는 `temp`의 `[left, right]` 위치만 쓰고 읽으며, 동시에 실행되는 스레드들의 범위는 서로 겹치지 않는다. 한 범위를 병합하는 시점에는 그 안의 하위 범위를 맡았던 스레드가 이미 조인되어 끝났으므로, 같은 위치를 동시에 쓰는 일이 없다.

   </details>

3. **IDX의 주소:** `N = 1024`일 때 `C[IDX(2, 3)]`은 배열 `C`의 시작에서 몇 바이트 떨어져 있는지 쓰라.

   <details>
   <summary>정답</summary>

   `IDX(2, 3) = 2 * 1024 + 3 = 2051`이고 double은 8바이트이므로 $2051 \times 8 = 16408$바이트 떨어져 있다.

   </details>
