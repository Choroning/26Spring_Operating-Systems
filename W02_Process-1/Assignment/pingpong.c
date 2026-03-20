/**
 * @file        pingpong.c
 * @brief       Two processes exchange a byte through pipes (ping-pong)
 * @author      Cheolwon Park
 * @date        2026-03-13
 */

/*
 * pingpong.c - Two processes exchange a byte through pipes
 *
 * Based on XV6 textbook Ch 1 Exercise 1
 *
 * Behavior:
 *   1. Parent sends 1 byte ("p") to child (ping)
 *   2. Child receives byte, prints it, and sends 1 byte ("p") back to parent (pong)
 *   3. Parent receives byte and prints it
 *   4. Measures and prints round-trip time
 *
 * Expected output format:
 *   <pid>: received ping
 *   <pid>: received pong
 *   Round-trip time: X.XXX us
 *
 * Compile: gcc -Wall -o pingpong pingpong.c
 * Run:     ./pingpong
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

/* Return current time in microseconds */
static long get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000L + tv.tv_usec;
}

int main(void)
{
    /*
     * Two pipes are needed for bidirectional communication.
     * parent_to_child: parent -> child direction
     * child_to_parent: child -> parent direction
     */
    int parent_to_child[2];
    int child_to_parent[2];

    /* TODO 1: Create 2 pipes. */
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
        /* ===== Child Process ===== */

        /* TODO 2: Close unused pipe fds. */
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        /* TODO 3: Read 1 byte from parent. */
        char buf;
        read(parent_to_child[0], &buf, 1);
        printf("%d: received ping\n", getpid());

        /* TODO 4: Send 1 byte to parent. */
        write(child_to_parent[1], &buf, 1);

        /* TODO 5: Close used pipe fds and exit with exit(0). */
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        exit(0);
    } else {
        /* ===== Parent Process ===== */

        /* TODO 6: Close unused pipe fds. */
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        long start_time = get_time_us();

        /* TODO 7: Send 1 byte to child (ping). */
        char buf = 'p';
        write(parent_to_child[1], &buf, 1);

        /* TODO 8: Read 1 byte from child (pong). */
        read(child_to_parent[0], &buf, 1);
        printf("%d: received pong\n", getpid());

        long end_time = get_time_us();
        double elapsed = (double)(end_time - start_time);
        printf("Round-trip time: %.3f us\n", elapsed);

        /* TODO 9: Close used pipe fds and wait for child with wait(). */
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        wait(NULL);
    }

    return 0;
}
