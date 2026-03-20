/**
 * @file        minishell.c
 * @brief       A simple shell supporting pipes and I/O redirection
 * @author      Cheolwon Park
 * @date        2026-03-13
 */

/*
 * minishell.c - Simple shell implementation
 *
 * Supported features:
 *   - Single command execution: ls -l
 *   - Pipe: cmd1 | cmd2
 *   - Input redirection: cmd < file
 *   - Output redirection: cmd > file
 *   - Exit: exit
 *
 * Compile: gcc -Wall -o minishell minishell.c
 * Run:     ./minishell
 *
 * Parsing is already implemented; only the following functions need to be added:
 *   1. execute_command()  - Execute a single command (with redirection)
 *   2. execute_pipe()     - Execute two commands connected by a pipe
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LINE    1024    /* Maximum input line length */
#define MAX_ARGS    64      /* Maximum number of arguments */

/* ===== Command Structure ===== */

struct command {
    char *argv[MAX_ARGS];   /* Argument list (NULL-terminated) */
    char *infile;           /* Input redirection filename, NULL if none */
    char *outfile;          /* Output redirection filename, NULL if none */
};


/* ===== Parsing (Implementation Complete) ===== */

/*
 * Trims leading and trailing whitespace from a string.
 * Returns a pointer to the trimmed string within the same buffer.
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
 * Parses a single command segment (no pipe) into a struct command.
 * Handles < and > redirection.
 *
 * Example: "sort < input.txt" -> argv={"sort", NULL}, infile="input.txt"
 * Example: "ls -l > out.txt"  -> argv={"ls", "-l", NULL}, outfile="out.txt"
 */
static int parse_command(char *line, struct command *cmd)
{
    memset(cmd, 0, sizeof(*cmd));

    int argc = 0;
    char *token = strtok(line, " \t");

    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            /* Input redirection */
            token = strtok(NULL, " \t");
            if (token == NULL) {
                fprintf(stderr, "minishell: syntax error near '<'\n");
                return -1;
            }
            cmd->infile = token;
        } else if (strcmp(token, ">") == 0) {
            /* Output redirection */
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
        return -1;  /* Empty command */
    }

    return 0;
}


/* ===== Implementation Area ===== */

/*
 * execute_command - Executes a single command.
 *
 * This function should:
 *   1. Create a child process with fork()
 *   2. In the child:
 *      a. If cmd->infile exists, set up input redirection (stdin to file)
 *      b. If cmd->outfile exists, set up output redirection (stdout to file)
 *      c. Execute the command with execvp()
 *      d. On exec failure, print error and exit(127)
 *   3. In the parent: wait for child termination with wait()
 *
 * Parameters:
 *   cmd - Parsed command structure (contains argv, infile, outfile)
 *
 * Return value:
 *   Child's exit status (WEXITSTATUS), or -1 on error
 */
static int execute_command(struct command *cmd)
{
    /* TODO 1: Create a child process with fork(). */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        /* TODO 2 (Child process): */

        /* a. Input redirection */
        if (cmd->infile != NULL) {
            int fd = open(cmd->infile, O_RDONLY);
            if (fd < 0) {
                perror(cmd->infile);
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        /* b. Output redirection */
        if (cmd->outfile != NULL) {
            int fd = open(cmd->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror(cmd->outfile);
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        /* c. Execute command */
        execvp(cmd->argv[0], cmd->argv);

        /* d. exec failure */
        perror("minishell");
        exit(127);
    }

    /* TODO 3 (Parent process): */
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return -1;
}

/*
 * execute_pipe - Executes two commands connected by a pipe.
 *
 * Implements the shell's "cmd1 | cmd2" behavior:
 *   - cmd1's stdout -> pipe -> cmd2's stdin
 *
 * This function should:
 *   1. Create a pipe with pipe()
 *   2. Fork child1 with fork() (runs cmd1)
 *      - dup2 stdout to the write end of the pipe
 *      - If cmd1->infile exists, also set up input redirection
 *      - Execute cmd1 with execvp
 *   3. Fork child2 with fork() (runs cmd2)
 *      - dup2 stdin to the read end of the pipe
 *      - If cmd2->outfile exists, also set up output redirection
 *      - Execute cmd2 with execvp
 *   4. In parent, close both ends of pipe + wait for both children
 *
 * Parameters:
 *   cmd1 - Left-side command of the pipe
 *   cmd2 - Right-side command of the pipe
 *
 * Return value:
 *   cmd2's exit status (WEXITSTATUS), or -1 on error
 */
static int execute_pipe(struct command *cmd1, struct command *cmd2)
{
    /* TODO 4: Create a pipe with pipe(). */
    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return -1;
    }

    /* TODO 5: Fork child1 with fork() (runs cmd1). */
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

    /* TODO 6: Fork child2 with fork() (runs cmd2). */
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

    /* TODO 7: Clean up in parent. */
    close(fd[0]);
    close(fd[1]);

    int status;
    waitpid(pid1, NULL, 0);
    waitpid(pid2, &status, 0);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return -1;
}


/* ===== Main Loop (Implementation Complete) ===== */

int main(void)
{
    char line[MAX_LINE];

    while (1) {
        /* Print prompt */
        printf("minishell> ");
        fflush(stdout);

        /* Read input */
        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;  /* EOF (Ctrl-D) */
        }

        /* Remove whitespace */
        char *input = trim(line);
        if (*input == '\0')
            continue;  /* Empty line */

        /* Built-in command handling: exit */
        if (strcmp(input, "exit") == 0)
            break;

        /* Check for pipe */
        char *pipe_pos = strchr(input, '|');

        if (pipe_pos != NULL) {
            /* Split at pipe character */
            *pipe_pos = '\0';
            char *left = input;
            char *right = pipe_pos + 1;

            /* Create copies for strtok (strtok modifies the string) */
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
            /* Single command */
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
