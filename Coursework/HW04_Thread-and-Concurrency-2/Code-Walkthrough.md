# HW4 Code Walkthrough — Implicit Threading

[Repository Guide](../../README.md) | [English](Code-Walkthrough.md) | [한국어](Code-Walkthrough.ko.md)

> **Last Updated:** 2026-11-16

> **Learning Objectives:**
> 1. Describe the functions of `mergesort.c` and `matmul.c` and distinguish the parts given in the starter code from the parts written for the assignment
> 2. Trace how `merge()` combines two sorted subarrays using a temporary array
> 3. Explain why the parallel merge sort is correct even though several threads share the same arrays
> 4. Explain the one-dimensional storage of the matrices and the memory access order of the three matrix multiplication functions

---

## Table of Contents

- [1. File Layout](#1-file-layout)
- [2. mergesort.c](#2-mergesortc)
  - [2.1 Includes, Constants, and the Argument Structure](#21-includes-constants-and-the-argument-structure)
  - [2.2 merge](#22-merge)
  - [2.3 merge_sort_sequential](#23-merge_sort_sequential)
  - [2.4 merge_sort_parallel](#24-merge_sort_parallel)
  - [2.5 get_time and is_sorted](#25-get_time-and-is_sorted)
  - [2.6 main](#26-main)
- [3. matmul.c](#3-matmulc)
  - [3.1 Includes, Global Matrices, and IDX](#31-includes-global-matrices-and-idx)
  - [3.2 Initialization Functions](#32-initialization-functions)
  - [3.3 The Three Multiplication Functions](#33-the-three-multiplication-functions)
  - [3.4 verify and main](#34-verify-and-main)
- [4. Tracing merge](#4-tracing-merge)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. File Layout

> **Try It First:** This document contains the complete code and line-by-line explanations. Solve the problems in the [assignment handout](Original/Assignment.md) yourself before reading on.

The companion document [HW4 Assignment Explanation](Assignment-Explanation.md) summarizes the requirements, the background, the implementation approach, and the experimental results and discussion. This document reads `mergesort.c` (179 lines) and `matmul.c` (145 lines) in `Solution/` from beginning to end. Both files replace the TODO comments of the distributed `Original/skeleton/` with code, and all other lines are the same as the starter code.

| File | Lines | Part | Relation to the starter code |
|:-----|:-----|:-----|:-----|
| `mergesort.c` | 1-65 | Header comment, includes, constants, `struct sort_arg`, `merge()`, `merge_sort_sequential()` | Unchanged |
| `mergesort.c` | 67-110 | Global `max_depth`, `merge_sort_parallel()` | Body lines 86-107 written |
| `mergesort.c` | 112-179 | `get_time()`, `is_sorted()`, `main()` | Unchanged |
| `matmul.c` | 1-50 | Header comment, includes, global matrices, `IDX`, initialization functions, `matmul_sequential()` | Unchanged |
| `matmul.c` | 52-62 | `matmul_openmp_basic()` | Body written |
| `matmul.c` | 64-78 | `matmul_openmp_optimized()` | Body written |
| `matmul.c` | 80-145 | `verify()`, `main()` | Unchanged |

*Table 1. Structure of the two source files and their relation to the starter code*

---

<br>

## 2. mergesort.c

### 2.1 Includes, Constants, and the Argument Structure

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

`DEFAULT_SIZE` is the default array size, 10 million, and `DEFAULT_DEPTH` is the default depth threshold, 3. `struct sort_arg` is the argument passed to the thread function; it holds pointers to the array to sort and the temporary array, both ends of the subrange (`left` and `right`, both inclusive), and the current recursion depth. All threads point to the same `array` and `temp` but handle only ranges that do not overlap.

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

Line 38 copies the range to merge, `array[left..right]`, to the same positions in `temp`. Then `i` points into the left part `temp[left..mid]`, `j` into the right part `temp[mid+1..right]`, and `k` to the position in `array` where the result is written. Lines 41-46 write the smaller of the two front elements to `array`, writing the left one first when they are equal (`<=`), so the original order of equal values is kept and the sort is stable. Lines 47-50 copy the remaining side as is. Because only the same positions of `temp` are used, threads merging non-overlapping ranges do not conflict even though they share `temp`.

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

If there is at most one element (`left >= right`), the range is already sorted, so the function returns. `mid = left + (right - left) / 2` on line 61 has the same value as `(left + right) / 2` but prevents `left + right` from exceeding the range of `int`. The two halves are sorted recursively and merged. The time complexity is $O(n \log n)$.

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

The global variable `max_depth` on line 74 is set by `main()` from a command-line argument, and all threads only read it. Lines 78-81 move the values of the argument structure into local variables. Lines 86-107 are the written part and follow exactly the five steps listed in the starter code's TODO comment. When the depth reaches the threshold, the function falls back to the sequential sort (lines 87-90); otherwise it computes the middle, builds the left-half argument `left_arg`, and passes it to a new thread (lines 95-97). It sorts the right half by calling itself directly with `right_arg` (lines 100-101), waits for the left thread with `pthread_join` (line 104), and merges (line 107).

There are three reasons this function is correct. First, the left thread and the current thread write only the non-overlapping ranges `[left, mid]` and `[mid+1, right]`. Second, the merge happens after `pthread_join` returns, so the current thread is guaranteed to see the results written by the left thread. Third, `left_arg` is a local variable on the stack, but since this function returns only after the join, it does not disappear while the new thread uses it. The return values of `pthread_create` and `pthread_join` are not checked. If thread creation failed, the left half would remain unsorted and the join would be performed on an invalid `tid`, so a robust implementation sorts the left half in the current thread when creation fails.

### 2.5 get_time and is_sorted

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

`get_time()` returns the current time measured with the monotonic clock in seconds. `is_sorted()` is a helper that compares neighboring elements to check whether the array is in ascending order, but `main()` does not call it; it verifies by comparing with the sequential result using `memcmp()`. Checking equality with the sequential result confirms both that the array is sorted and that it has the same elements, so it is the stronger check.

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

Lines 131-132 read the array size and the depth threshold from the command-line arguments, and line 136 prints the maximum thread count as `1 << max_depth`. Lines 139-141 allocate the original, working, and temporary arrays, and lines 144-146 generate random values in `[0, 10n)` with the fixed seed 42. Both the sequential sort (lines 149-154) and the parallel sort (lines 161-168) start from a copy of the same original, and the sequential result is kept in `expected`. The initial argument on line 162 is the full range `[0, n - 1]` with depth 0, and on line 164 the main thread calls `merge_sort_parallel()` directly. Line 171 compares the two results and prints `CORRECT` or `MISMATCH!`, returning 0 as the exit code if they match.

---

<br>

## 3. matmul.c

### 3.1 Includes, Global Matrices, and IDX

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

`<omp.h>` declares `omp_get_max_threads()` and `omp_get_wtime()`. The three matrices are stored not as two-dimensional arrays but as one-dimensional arrays of $N^2$ elements, and `IDX(i, j)` computes the position `i * N + j` of row i, column j. This is row-major storage, in which the elements of a row are contiguous in memory, so increasing j by 1 advances the address by 8 bytes, and increasing i by 1 advances it by `8N` bytes.

### 3.2 Initialization Functions

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

`init_matrices()` fills A and B with values from 0.0 to 9.9 in steps of 0.1 using seed 42. `zero_C()` clears C to 0 so that each version accumulates its result afresh. All three multiplication functions accumulate with `C[...] +=`, so if C were not cleared before a run, the result would be added to that of the previous version.

### 3.3 The Three Multiplication Functions

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

The sequential version (lines 44-50) is the i-j-k loop given in the starter code. In the innermost k loop, `A[IDX(i, k)]` has stride 1, but `B[IDX(k, j)]` has stride N.

The basic OpenMP version (lines 55-62) puts `#pragma omp parallel for` above the same loop. The directive distributes the iterations of the immediately following `for` loop, the i loop, among the threads. The loop variables `j` and `k` are declared inside the `for` statements, so each thread gets its own copies, and each thread writes only the `C` elements of different rows i, so there is no race. The schedule is not specified, so the runtime's default applies; since every row has the same amount of work, even a static division balances the load.

The optimized version (lines 71-78) swaps the k and j loops. In the innermost j loop, `A[IDX(i, k)]` is constant, and `B[IDX(k, j)]` and `C[IDX(i, j)]` advance with stride 1. The order of the terms added to each `C[i][j]` is k = 0, 1, ..., the same as in i-j-k, so the computed values are the same as in the sequential version. The header comment (line 7) says "loop reordering i-k-j + collapse", but the function does not use `collapse(2)`; it only reorders the loops.

### 3.4 verify and main

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

`verify()` compares the two results element by element and treats a difference greater than $10^{-6}$ as a failure. Even with the same order of additions, rounding can differ slightly if the compiler fuses a multiplication and an addition into one FMA or vectorizes the loop, so a tolerance is used instead of exact equality. `main()` reads the matrix size from the arguments and prints the number of threads OpenMP will use with `omp_get_max_threads()`. Each of the three versions clears C with `zero_C()` and is timed with `omp_get_wtime()`, and the sequential result is copied to `C_ref` to verify the two OpenMP versions. The number in parentheses in the output is the speedup relative to the sequential time.

---

<br>

## 4. Tracing merge

Consider the step just before the last one in the handout's example, merging `[3, 8]` and `[1, 5]`. With `array[0..3] = {3, 8, 1, 5}`, `left = 0`, `mid = 1`, and `right = 3`, line 38 copies these four elements to `temp[0..3]`, and the merge proceeds as follows.

| Step | i, j | Comparison | Value written | array[0..3] |
|:-----|:-----|:-----|:-----|:-----|
| 1 | 0, 2 | `temp[0]=3 <= temp[2]=1` false | 1 (`j++`) | 1, 8, 1, 5 |
| 2 | 0, 3 | `3 <= temp[3]=5` true | 3 (`i++`) | 1, 3, 1, 5 |
| 3 | 1, 3 | `temp[1]=8 <= 5` false | 5 (`j++`) | 1, 3, 5, 5 |
| 4 | 1, 4 | First loop ends since `j > right`; copy the rest of the left | 8 (`i++`) | 1, 3, 5, 8 |

*Table 2. Merging [3, 8] and [1, 5] with merge*

Because comparisons are made in `temp` while results are written to `array`, the result is not corrupted even when an overwritten element (such as `array[2]` after step 1) is still pending comparison.

---

<br>

## Summary

| Concept | Key Summary |
|:-----|:-----|
| Written parts | Only the body of `merge_sort_parallel()` (lines 86-107) and the bodies of the two OpenMP matrix multiplication functions were written; the rest is the same as the starter code. |
| merge | It copies the range to `temp` and writes to `array` while comparing the front elements of the two parts, and the `<=` comparison makes the sort stable. |
| Correctness of the parallel sort | The threads write only non-overlapping ranges, the merge happens after the join, and `left_arg` lives until the join. |
| Verification | Merge sort compares with the sequential result using `memcmp()`, and matrix multiplication compares with the sequential result within a tolerance of $10^{-6}$; `is_sorted()` is unused. |
| Matrix storage | The matrices are row-major one-dimensional arrays with `IDX(i, j) = i * N + j`, so increasing the column index by 1 moves 8 bytes and increasing the row index by 1 moves `8N` bytes. |
| OpenMP functions | The basic version splits the i loop by rows, and the optimized version uses the same directive with the i-k-j order to read B and C with stride 1, without `collapse(2)`. |

---

<br>

## Self-Check Questions

1. **Lifetime of left_arg:** State what would go wrong with `left_arg` if `merge_sort_parallel()` returned before calling `pthread_join`.

   <details>
   <summary>Answer</summary>

   `left_arg` is a local variable on the function's stack, so once the function returns, other calls can reuse that memory. If the new thread had not yet read fields such as `sa->left`, it would sort the wrong range. Moreover, without the join there would be no guarantee that the left half was sorted at the time of the merge.

   </details>

2. **Sharing temp:** Explain why there are no conflicts even though all threads use the same `temp` array.

   <details>
   <summary>Answer</summary>

   `merge()` writes and reads only positions `[left, right]` of `temp`, and the ranges of concurrently running threads do not overlap. By the time a range is merged, the threads that handled its subranges have already been joined and finished, so no position is written concurrently.

   </details>

3. **Address from IDX:** With `N = 1024`, state how many bytes `C[IDX(2, 3)]` is from the start of the array `C`.

   <details>
   <summary>Answer</summary>

   `IDX(2, 3) = 2 * 1024 + 3 = 2051`, and a double is 8 bytes, so it is $2051 \times 8 = 16408$ bytes away.

   </details>
