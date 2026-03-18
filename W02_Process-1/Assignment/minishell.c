/**
 * @file        minishell.c
 * @brief       A simple shell supporting pipes and I/O redirection
 * @author      Cheolwon Park
 * @date        2026-03-13
 */

/*
 * minishell.c - 간단한 셸 구현
 *
 * 지원 기능:
 *   - 단일 명령어 실행: ls -l
 *   - 파이프: cmd1 | cmd2
 *   - 입력 리다이렉션: cmd < file
 *   - 출력 리다이렉션: cmd > file
 *   - 종료: exit
 *
 * 컴파일: gcc -Wall -o minishell minishell.c
 * 실행:   ./minishell
 *
 * 파싱은 이미 구현되어 있으며, 추가로 다음 함수만 구현하면 됩니다:
 *   1. execute_command()  - 단일 명령어 실행 (리다이렉션 포함)
 *   2. execute_pipe()     - 파이프로 연결된 두 명령어 실행
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE    1024    /* 최대 입력 줄 길이 */
#define MAX_ARGS    64      /* 최대 인자 개수 */

/* ===== 명령어 구조체 ===== */

struct command {
    char *argv[MAX_ARGS];   /* 인자 목록 (NULL 종료) */
    char *infile;           /* 입력 리다이렉션 파일명, 없으면 NULL */
    char *outfile;          /* 출력 리다이렉션 파일명, 없으면 NULL */
};


/* ===== 파싱 (구현 완료) ===== */

/*
 * 문자열의 앞뒤 공백을 제거합니다.
 * 같은 버퍼 내에서 잘라낸 문자열의 포인터를 반환합니다.
 */
static char *trim(char *s)
{
    while (*s == ' ' || *s == '\t')
        s++;
    if (*s == '\0')
        return s;
    char *end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n'))
        *end-- = '\0';
    return s;
}

/*
 * 단일 명령어 세그먼트(파이프 없음)를 struct command로 파싱합니다.
 * < 및 > 리다이렉션을 처리합니다.
 *
 * 예시: "sort < input.txt" → argv={"sort", NULL}, infile="input.txt"
 * 예시: "ls -l > out.txt"  → argv={"ls", "-l", NULL}, outfile="out.txt"
 */
static int parse_command(char *line, struct command *cmd)
{
    memset(cmd, 0, sizeof(*cmd));

    int argc = 0;
    char *token = strtok(line, " \t");

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            /* 입력 리다이렉션 */
            token = strtok(NULL, " \t");
            if (token == NULL) {
                fprintf(stderr, "minishell: syntax error near '<'\n");
                return -1;
            }
            cmd->infile = token;
        } else if (strcmp(token, ">") == 0) {
            /* 출력 리다이렉션 */
            token = strtok(NULL, " \t");
            if (token == NULL) {
                fprintf(stderr, "minishell: syntax error near '>'\n");
                return -1;
            }
            cmd->outfile = token;
        } else {
            if (argc >= MAX_ARGS - 1) {
                fprintf(stderr, "minishell: too many arguments\n");
                return -1;
            }
            cmd->argv[argc++] = token;
        }
        token = strtok(NULL, " \t");
    }

    cmd->argv[argc] = NULL;

    if (argc == 0) {
        return -1;  /* 빈 명령어 */
    }

    return 0;
}


/* ===== 구현 영역 ===== */

/*
 * execute_command - 단일 명령어를 실행합니다.
 *
 * 이 함수는 다음을 수행해야 합니다:
 *   1. fork()로 자식 프로세스 생성
 *   2. 자식에서:
 *      a. cmd->infile이 있으면 입력 리다이렉션 설정 (stdin을 파일로)
 *      b. cmd->outfile이 있으면 출력 리다이렉션 설정 (stdout을 파일로)
 *      c. execvp()로 명령어 실행
 *      d. exec 실패 시 에러 출력 후 exit(127)
 *   3. 부모에서: wait()으로 자식 종료 대기
 *
 * 매개변수:
 *   cmd - 파싱된 명령어 구조체 (argv, infile, outfile 포함)
 *
 * 반환값:
 *   자식의 exit status (WEXITSTATUS), 또는 에러 시 -1
 */
static int execute_command(struct command *cmd)
{
    /* TODO 1: fork()로 자식 프로세스를 생성하세요. */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        /* TODO 2 (자식 프로세스): */

        /* a. 입력 리다이렉션 */
        if (cmd->infile != NULL) {
            int fd = open(cmd->infile, O_RDONLY);
            if (fd < 0) {
                perror(cmd->infile);
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        /* b. 출력 리다이렉션 */
        if (cmd->outfile != NULL) {
            int fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror(cmd->outfile);
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        /* c. 명령어 실행 */
        execvp(cmd->argv[0], cmd->argv);

        /* d. exec 실패 */
        perror("minishell");
        exit(127);
    }

    /* TODO 3 (부모 프로세스): */
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return -1;
}

/*
 * execute_pipe - 파이프로 연결된 두 명령어를 실행합니다.
 *
 * 셸의 "cmd1 | cmd2" 동작을 구현합니다:
 *   - cmd1의 stdout → pipe → cmd2의 stdin
 *
 * 이 함수는 다음을 수행해야 합니다:
 *   1. pipe()로 파이프 생성
 *   2. fork()로 자식1 생성 (cmd1 실행)
 *      - stdout을 pipe의 쓰기 끝으로 dup2
 *      - cmd1->infile이 있으면 입력 리다이렉션도 설정
 *      - execvp로 cmd1 실행
 *   3. fork()로 자식2 생성 (cmd2 실행)
 *      - stdin을 pipe의 읽기 끝으로 dup2
 *      - cmd2->outfile이 있으면 출력 리다이렉션도 설정
 *      - execvp로 cmd2 실행
 *   4. 부모에서 pipe 양쪽 닫기 + 자식 2개 모두 wait
 *
 * 매개변수:
 *   cmd1 - 파이프 왼쪽 명령어
 *   cmd2 - 파이프 오른쪽 명령어
 *
 * 반환값:
 *   cmd2의 exit status (WEXITSTATUS), 또는 에러 시 -1
 */
static int execute_pipe(struct command *cmd1, struct command *cmd2)
{
    /* TODO 4: pipe()로 파이프를 생성하세요. */
    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return -1;
    }

    /* TODO 5: fork()로 자식1을 생성하세요 (cmd1 실행). */
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return -1;
    }

    if (pid1 == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        if (cmd1->infile != NULL) {
            int infd = open(cmd1->infile, O_RDONLY);
            if (infd < 0) {
                perror(cmd1->infile);
                exit(1);
            }
            dup2(infd, STDIN_FILENO);
            close(infd);
        }

        execvp(cmd1->argv[0], cmd1->argv);
        perror("minishell");
        exit(127);
    }

    /* TODO 6: fork()로 자식2를 생성하세요 (cmd2 실행). */
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return -1;
    }

    if (pid2 == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        if (cmd2->outfile != NULL) {
            int outfd = open(cmd2->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (outfd < 0) {
                perror(cmd2->outfile);
                exit(1);
            }
            dup2(outfd, STDOUT_FILENO);
            close(outfd);
        }

        execvp(cmd2->argv[0], cmd2->argv);
        perror("minishell");
        exit(127);
    }

    /* TODO 7: 부모에서 정리하세요. */
    close(fd[0]);
    close(fd[1]);

    int status;
    waitpid(pid1, NULL, 0);
    waitpid(pid2, &status, 0);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return -1;
}


/* ===== 메인 루프 (구현 완료) ===== */

int main(void)
{
    char line[MAX_LINE];

    while (1) {
        /* 프롬프트 출력 */
        printf("minishell> ");
        fflush(stdout);

        /* 입력 읽기 */
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;  /* EOF (Ctrl-D) */
        }

        /* 공백 제거 */
        char *input = trim(line);
        if (*input == '\0')
            continue;  /* 빈 줄 */

        /* 내장 명령어 처리: exit */
        if (strcmp(input, "exit") == 0)
            break;

        /* 파이프 확인 */
        char *pipe_pos = strchr(input, '|');

        if (pipe_pos != NULL) {
            /* 파이프 문자에서 분리 */
            *pipe_pos = '\0';
            char *left = input;
            char *right = pipe_pos + 1;

            /* strtok용 복사본 생성 (strtok는 문자열을 수정함) */
            char left_copy[MAX_LINE], right_copy[MAX_LINE];
            strncpy(left_copy, left, sizeof(left_copy) - 1);
            left_copy[sizeof(left_copy) - 1] = '\0';
            strncpy(right_copy, right, sizeof(right_copy) - 1);
            right_copy[sizeof(right_copy) - 1] = '\0';

            struct command cmd1, cmd2;
            if (parse_command(left_copy, &cmd1) < 0)
                continue;
            if (parse_command(right_copy, &cmd2) < 0)
                continue;

            execute_pipe(&cmd1, &cmd2);
        } else {
            /* 단일 명령어 */
            char copy[MAX_LINE];
            strncpy(copy, input, sizeof(copy) - 1);
            copy[sizeof(copy) - 1] = '\0';

            struct command cmd;
            if (parse_command(copy, &cmd) < 0)
                continue;

            execute_command(&cmd);
        }
    }

    return 0;
}
