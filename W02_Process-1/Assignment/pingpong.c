/**
 * @file        pingpong.c
 * @brief       Two processes exchange a byte through pipes (ping-pong)
 * @author      Cheolwon Park
 * @date        2026-03-13
 */

/*
 * pingpong.c - 두 프로세스가 pipe를 통해 1바이트를 주고받는 프로그램
 *
 * XV6 교재 Ch 1 Exercise 1 기반
 *
 * 동작:
 *   1. 부모가 자식에게 1바이트("p")를 보냄 (ping)
 *   2. 자식이 바이트를 받고 출력, 부모에게 1바이트("p")를 다시 보냄 (pong)
 *   3. 부모가 바이트를 받고 출력
 *   4. 왕복 시간을 측정하여 출력
 *
 * 예상 출력 형식:
 *   <pid>: received ping
 *   <pid>: received pong
 *   Round-trip time: X.XXX us
 *
 * 컴파일: gcc -Wall -o pingpong pingpong.c
 * 실행:  ./pingpong
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

/* 현재 시각을 마이크로초 단위로 반환 */
static long get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000L + tv.tv_usec;
}

int main(void)
{
    /*
     * 양방향 통신을 위해 pipe 2개가 필요합니다.
     * parent_to_child: 부모 → 자식 방향
     * child_to_parent: 자식 → 부모 방향
     */
    int parent_to_child[2];
    int child_to_parent[2];

    /* TODO 1: pipe 2개를 생성하세요. */
    if (pipe(parent_to_child) < 0) {
        perror("pipe");
        exit(1);
    }
    if (pipe(child_to_parent) < 0) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        /* ===== 자식 프로세스 ===== */

        /* TODO 2: 사용하지 않는 pipe fd를 닫으세요. */
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        /* TODO 3: 부모로부터 1바이트를 읽으세요. */
        char buf;
        read(parent_to_child[0], &buf, 1);
        printf("%d: received ping\n", getpid());

        /* TODO 4: 부모에게 1바이트를 보내세요. */
        write(child_to_parent[1], &buf, 1);

        /* TODO 5: 사용한 pipe fd를 닫고 exit(0)으로 종료하세요. */
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        exit(0);
    } else {
        /* ===== 부모 프로세스 ===== */

        /* TODO 6: 사용하지 않는 pipe fd를 닫으세요. */
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        long start_time = get_time_us();

        /* TODO 7: 자식에게 1바이트를 보내세요 (ping). */
        char buf = 'p';
        write(parent_to_child[1], &buf, 1);

        /* TODO 8: 자식으로부터 1바이트를 읽으세요 (pong). */
        read(child_to_parent[0], &buf, 1);
        printf("%d: received pong\n", getpid());

        long end_time = get_time_us();
        double elapsed = (double)(end_time - start_time);
        printf("Round-trip time: %.3f us\n", elapsed);

        /* TODO 9: 사용한 pipe fd를 닫고 wait()으로 자식을 기다리세요. */
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        wait(NULL);
    }

    return 0;
}
