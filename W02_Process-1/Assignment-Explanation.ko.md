# 2주차 과제 해설 — pingpong & minishell

> **최종 수정일:** 2026-03-21
>
> 강의 연계: Lecture (Ch 3.1–3.3 프로세스), Lab (실습 3–4 파이프/dup2)

---

## 파일 구조 및 사용법

```plaintext
W02_Process-1/Assignment/
├── pingpong.c                  # 과제 1 — 두 프로세스가 pipe로 1바이트를 주고받는 프로그램
├── minishell.c                 # 과제 2 — 파이프 및 I/O 리다이렉션을 지원하는 간단한 셸
├── test_pingpong.sh            # pingpong.c 자동 채점 스크립트
├── test_minishell.sh           # minishell.c 자동 채점 스크립트
└── Assignment-Explanation.md   # 과제 해설 문서 (본 파일)
```

### 빌드 및 실행

```bash
# pingpong
gcc -Wall -o pingpong pingpong.c
./pingpong

# minishell
gcc -Wall -o minishell minishell.c
./minishell
```

### 테스트

```bash
# 같은 디렉토리에서 실행 (기본값: 같은 경로의 .c 파일 컴파일)
./test_pingpong.sh
./test_minishell.sh

# 다른 경로의 소스 파일을 지정할 수도 있음
./test_pingpong.sh /path/to/pingpong.c
./test_minishell.sh /path/to/minishell.c
```

---

## 목차

- [1. 과제 개요](#1-과제-개요)
  - [1.1 이번 과제에서 다루는 시스템 콜](#11-이번-과제에서-다루는-시스템-콜)
- [2. 과제 1 — pingpong.c 해설](#2-과제-1--pingpongc-해설)
  - [2.1 무엇을 구현하는가?](#21-무엇을-구현하는가)
  - [2.2 왜 파이프가 2개 필요한가?](#22-왜-파이프가-2개-필요한가)
  - [2.3 TODO 1: 파이프 2개 생성](#23-todo-1-파이프-2개-생성)
  - [2.4 TODO 2, 6: 사용하지 않는 fd 닫기](#24-todo-2-6-사용하지-않는-fd-닫기)
  - [2.5 TODO 3, 4, 5: 자식 프로세스의 동작](#25-todo-3-4-5-자식-프로세스의-동작)
  - [2.6 TODO 7, 8, 9: 부모 프로세스의 동작](#26-todo-7-8-9-부모-프로세스의-동작)
  - [2.7 전체 데이터 흐름 정리](#27-전체-데이터-흐름-정리)
  - [2.8 핵심 학습 포인트](#28-핵심-학습-포인트)
- [3. 과제 2 — minishell.c 해설 (execute_command)](#3-과제-2--minishellc-해설-execute_command)
  - [3.1 무엇을 구현하는가?](#31-무엇을-구현하는가)
  - [3.2 주어진 자료구조 이해](#32-주어진-자료구조-이해)
  - [3.3 TODO 1: fork()로 자식 생성](#33-todo-1-fork로-자식-생성)
  - [3.4 TODO 2a: 입력 리디렉션](#34-todo-2a-입력-리디렉션)
  - [3.5 TODO 2b: 출력 리디렉션](#35-todo-2b-출력-리디렉션)
  - [3.6 TODO 2c, 2d: 명령어 실행](#36-todo-2c-2d-명령어-실행)
  - [3.7 TODO 3: 부모의 대기](#37-todo-3-부모의-대기)
  - [3.8 execute_command 전체 흐름 요약](#38-execute_command-전체-흐름-요약)
- [4. 과제 2 — minishell.c 해설 (execute_pipe)](#4-과제-2--minishellc-해설-execute_pipe)
  - [4.1 파이프가 무엇을 하는가?](#41-파이프가-무엇을-하는가)
  - [4.2 TODO 4: 파이프 생성](#42-todo-4-파이프-생성)
  - [4.3 TODO 5: 자식1 (cmd1 실행)](#43-todo-5-자식1-cmd1-실행)
  - [4.4 TODO 6: 자식2 (cmd2 실행)](#44-todo-6-자식2-cmd2-실행)
  - [4.5 TODO 7: 부모에서 정리](#45-todo-7-부모에서-정리)
  - [4.6 execute_pipe 전체 흐름 요약](#46-execute_pipe-전체-흐름-요약)
  - [4.7 파이프 + 리디렉션 조합](#47-파이프--리디렉션-조합)
- [5. 핵심 개념 정리 및 시험 대비 포인트](#5-핵심-개념-정리-및-시험-대비-포인트)
  - [5.1 시스템 콜 조합 패턴 총정리](#51-시스템-콜-조합-패턴-총정리)
  - [5.2 fd 관리 규칙](#52-fd-관리-규칙)
  - [5.3 이 과제와 강의 내용의 연결](#53-이-과제와-강의-내용의-연결)
  - [5.4 시험 대비 — 자주 출제되는 포인트](#54-시험-대비--자주-출제되는-포인트)
- [요약](#요약)

---

<br>

## 1. 과제 개요

### 1.1 이번 과제에서 다루는 시스템 콜

| 시스템 콜 | 역할 | 강의 연계 |
|:----------|:-----|:----------|
| **fork()** | 현재 프로세스의 복사본(자식 프로세스) 생성 | Lecture: 프로세스 생성 (Ch 3.3.1) |
| **exec()** | 프로세스 이미지를 새 프로그램으로 교체 | Lecture: fork() 이후의 exec() |
| **wait()** | 자식 프로세스의 종료를 대기; 좀비(Zombie) 방지 | Lecture: 프로세스 종료 (Ch 3.3.2) |
| **pipe()** | 단방향 IPC 채널(커널 버퍼) 생성 | Lab: 실습 3 — pipe() |
| **dup2()** | 파일 디스크립터(FD)를 복제하여 I/O 리디렉션 | Lab: 실습 4 — dup2() |
| **read() / write()** | 파일 디스크립터를 통한 데이터 읽기/쓰기 | Lab: 실습 3 — pipe() |
| **close()** | 파일 디스크립터 닫기 | Lab: 사용하지 않는 fd는 반드시 닫기 |

**과제 구성:**
- **pingpong** (기본): 두 프로세스가 파이프로 1바이트를 주고받는 프로그램
- **minishell** (중급): 파이프와 I/O 리디렉션을 지원하는 간단한 셸

---

<br>

## 2. 과제 1 — pingpong.c 해설

### 2.1 무엇을 구현하는가?

**목표:** 부모 프로세스와 자식 프로세스가 파이프(Pipe)를 통해 1바이트를 **왕복**으로 주고받고, 왕복 시간(Round-trip Time)을 측정한다.

```text
부모 프로세스 (Parent)                   자식 프로세스 (Child)
     │                                       │
     ├── 'p' 전송 (ping) ───────────────────→ │
     │                                       ├── "received ping" 출력
     │ ←─────────────────────── 'p' 전송 ──── │
     ├── "received pong" 출력                 │
     ├── 왕복 시간 출력                         │
     └── wait()으로 자식 회수                   └── exit(0)
```

**예상 출력:**
```
12345: received ping
12344: received pong
Round-trip time: 156.000 us
```

> **[컴퓨터네트워크 — RTT]** 왕복 시간(Round-trip Time, RTT)은 네트워크에서 패킷이 목적지까지 갔다가 돌아오는 데 걸리는 시간을 의미한다. 이 과제에서는 프로세스 간 파이프 통신의 RTT를 측정하는데, 이는 IPC(프로세스 간 통신)의 오버헤드를 파악하는 데 유용하다. 일반적으로 같은 호스트 내 파이프 RTT는 수백 마이크로초 수준이다.

### 2.2 왜 파이프가 2개 필요한가?

파이프(Pipe)는 **단방향(unidirectional)** 통신 채널이다. 한쪽 끝(`fd[1]`)으로 쓰고, 다른 쪽 끝(`fd[0]`)으로 읽는다.

**양방향 통신**을 하려면 파이프 2개가 필요하다:

```text
parent_to_child[2]:   부모가 [1]에 쓰기 → 자식이 [0]에서 읽기
child_to_parent[2]:   자식이 [1]에 쓰기 → 부모가 [0]에서 읽기
```

```c
int parent_to_child[2];   /* 부모 → 자식 방향 */
int child_to_parent[2];   /* 자식 → 부모 방향 */
```

| 파이프 | 쓰기 끝 `[1]` | 읽기 끝 `[0]` |
|:-------|:-------------|:-------------|
| `parent_to_child` | 부모가 사용 | 자식이 사용 |
| `child_to_parent` | 자식이 사용 | 부모가 사용 |

> **[Lab 실습 3]** 실습에서 배운 `pipe()` 함수가 `int fd[2]` 배열에 `fd[0]`(읽기 끝)과 `fd[1]`(쓰기 끝)을 채워주는 것을 기억할 것. 이 과제에서는 이 원리를 확장하여 **2개의 파이프**로 양방향 통신을 구현한다.

### 2.3 TODO 1: 파이프 2개 생성

```c
/* TODO 1: pipe 2개를 생성하세요. */
if (pipe(parent_to_child) < 0) {
    perror("pipe");
    exit(1);
}
if (pipe(child_to_parent) < 0) {
    perror("pipe");
    exit(1);
}
```

**핵심 포인트:**
- `pipe()` 호출 후 반환 값을 확인하여 **오류 처리**를 해야 한다.
- `pipe()` 실패 시 `-1`을 반환하고 `errno`를 설정한다.
- `perror()`는 `errno`에 해당하는 오류 메시지를 자동으로 출력해 준다.
- 파이프 생성은 `fork()` **이전에** 해야 한다 — 자식 프로세스가 부모의 파일 디스크립터 테이블을 상속받기 때문이다.

> **[Lecture]** 강의에서 배운 것처럼, `fork()`는 부모 프로세스의 **주소 공간의 복사본**을 만든다. 파일 디스크립터 테이블도 복사되므로, `fork()` 이전에 생성한 파이프의 fd를 부모와 자식이 모두 갖게 된다. 이것이 파이프를 통한 부모-자식 간 통신이 가능한 이유이다.

### 2.4 TODO 2, 6: 사용하지 않는 fd 닫기

`fork()` 후 부모와 자식 모두 4개의 fd를 갖고 있다. 각 프로세스에서 **사용하지 않는 끝**을 닫아야 한다.

**자식 프로세스 (TODO 2):**
```c
/* 자식은 parent_to_child에서 읽기만, child_to_parent에 쓰기만 한다 */
close(parent_to_child[1]);   /* 부모→자식 파이프의 쓰기 끝 (자식은 안 씀) */
close(child_to_parent[0]);   /* 자식→부모 파이프의 읽기 끝 (자식은 안 읽음) */
```

**부모 프로세스 (TODO 6):**
```c
/* 부모는 parent_to_child에 쓰기만, child_to_parent에서 읽기만 한다 */
close(parent_to_child[0]);   /* 부모→자식 파이프의 읽기 끝 (부모는 안 읽음) */
close(child_to_parent[1]);   /* 자식→부모 파이프의 쓰기 끝 (부모는 안 씀) */
```

> **[Lab 실습 3]** 실습에서 "항상 사용하지 않는 쪽을 닫을 것"이라는 규칙을 배웠다. 이유: (1) 쓰기 끝이 열려 있으면 읽는 쪽에서 **EOF를 수신할 수 없어** 프로그램이 영원히 블록된다. (2) 파일 디스크립터는 유한한 자원이므로 불필요한 fd는 즉시 해제해야 한다.

### 2.5 TODO 3, 4, 5: 자식 프로세스의 동작

```c
/* TODO 3: 부모로부터 1바이트를 읽으세요. */
char buf;
read(parent_to_child[0], &buf, 1);    /* 부모가 보낸 'p'를 읽음 */
printf("%d: received ping\n", getpid());

/* TODO 4: 부모에게 1바이트를 보내세요. */
write(child_to_parent[1], &buf, 1);   /* 받은 바이트를 다시 부모에게 보냄 */

/* TODO 5: 사용한 pipe fd를 닫고 exit(0)으로 종료하세요. */
close(parent_to_child[0]);
close(child_to_parent[1]);
exit(0);
```

**실행 흐름:**
1. `read()`는 부모가 데이터를 보낼 때까지 **블록(block)**됨 — 동기화 역할
2. 바이트를 읽으면 `"received ping"` 출력
3. 같은 바이트를 `write()`로 부모에게 돌려보냄 (pong)
4. 사용한 fd를 모두 닫고 `exit(0)`으로 정상 종료

> **[Lecture]** 자식 프로세스가 `exit(0)`으로 종료하면, 부모가 `wait()`를 호출할 때까지 **좀비 프로세스(Zombie Process)** 상태가 된다. 이 과제에서는 부모가 마지막에 `wait(NULL)`을 호출하여 좀비를 회수한다.

### 2.6 TODO 7, 8, 9: 부모 프로세스의 동작

```c
long start_time = get_time_us();      /* 시간 측정 시작 */

/* TODO 7: 자식에게 1바이트를 보내세요 (ping). */
char buf = 'p';
write(parent_to_child[1], &buf, 1);   /* 'p' 전송 */

/* TODO 8: 자식으로부터 1바이트를 읽으세요 (pong). */
read(child_to_parent[0], &buf, 1);    /* 자식의 응답을 읽음 */
printf("%d: received pong\n", getpid());

long end_time = get_time_us();        /* 시간 측정 종료 */
double elapsed = (double)(end_time - start_time);
printf("Round-trip time: %.3f us\n", elapsed);

/* TODO 9: 사용한 pipe fd를 닫고 wait()으로 자식을 기다리세요. */
close(parent_to_child[1]);
close(child_to_parent[0]);
wait(NULL);   /* 자식 프로세스 회수 — 좀비 방지 */
```

> **[Lecture — 문맥 교환]** 왕복 시간에는 **문맥 교환(Context Switch)** 오버헤드가 포함된다. 부모가 `write()` 후 자식이 `read()`로 깨어나려면 CPU가 부모에서 자식으로 전환되어야 하고, 자식이 `write()` 후 다시 부모가 `read()`로 깨어나려면 또 한 번 전환이 필요하다. 즉, 측정된 RTT = 데이터 복사 시간 + 문맥 교환 시간 × 2.

### 2.7 전체 데이터 흐름 정리

```text
시간 →

부모 프로세스                               자식 프로세스
━━━━━━━━━                                ━━━━━━━━━
pipe(parent_to_child)
pipe(child_to_parent)
fork() ──────────────────────────────────→ [생성됨]
close(parent_to_child[0])          close(parent_to_child[1])
close(child_to_parent[1])          close(child_to_parent[0])
                                              │
start_time = get_time_us()                    │
write(parent_to_child[1], 'p') ───→ read(parent_to_child[0])
        │                            printf("received ping")
        │                         write(child_to_parent[1], 'p')
read(child_to_parent[0]) ←─────────
printf("received pong")                   close(...)
end_time = get_time_us()              exit(0) → [좀비 상태]
printf("Round-trip time: ...")                │
close(...)                                    │
wait(NULL) ──────────────────────────────→ [회수됨]
```

### 2.8 핵심 학습 포인트

| 학습 포인트 | 강의 연계 | 이 과제에서의 적용 |
|:-----------|:----------|:-----------------|
| **파이프는 단방향** | Lab: `pipe()` 실습 | 양방향 통신에 파이프 2개 사용 |
| **fork()는 fd를 상속** | Lecture: fork()의 동작 방식 | fork() 전에 pipe() 생성 → 부모·자식 모두 fd 보유 |
| **사용하지 않는 fd 닫기** | Lab: 파이프 규칙 | 각 프로세스에서 불필요한 2개의 fd를 close() |
| **read()의 블로킹** | Lab: 생산자-소비자 패턴 | 부모가 쓸 때까지 자식의 read()가 대기 → 자연스러운 동기화 |
| **wait()으로 좀비 방지** | Lecture: 좀비 프로세스 | 부모가 마지막에 wait(NULL) 호출 |
| **문맥 교환 오버헤드** | Lecture: 문맥 교환 | RTT에 문맥 교환 비용이 포함됨 |

---

<br>

## 3. 과제 2 — minishell.c 해설 (execute_command)

### 3.1 무엇을 구현하는가?

**목표:** `fork()`, `exec()`, `wait()`, `pipe()`, `dup2()`를 활용하여 **실제 셸(Shell)**의 핵심 동작을 구현한다.

| 기능 | 예시 | 사용 시스템 콜 |
|:-----|:-----|:-------------|
| 단일 명령어 실행 | `ls -l` | fork + execvp + wait |
| 출력 리디렉션 | `echo hello > file.txt` | open + dup2 + execvp |
| 입력 리디렉션 | `sort < data.txt` | open + dup2 + execvp |
| 파이프 | `ls \| wc -l` | pipe + fork×2 + dup2 + execvp |
| 종료 | `exit` | 루프 탈출 |

**이미 구현된 부분:** `main()` (입력 읽기, 파이프 분할, exit 처리), `parse_command()` (명령어 파싱, 리디렉션 파싱), `struct command`

**학생이 구현할 부분:** `execute_command()`, `execute_pipe()`

> **[Lecture — 셸의 동작 원리]** 강의에서 "UNIX 셸의 기본 동작 원리 = fork() + exec() + wait()의 반복"이라고 배웠다. 이 과제는 그 원리를 직접 코드로 구현하는 것이다.

### 3.2 주어진 자료구조 이해

```c
struct command {
    char *argv[MAX_ARGS];   /* 인자 목록 (NULL로 종료) */
    char *infile;           /* 입력 리디렉션 파일명, 없으면 NULL */
    char *outfile;          /* 출력 리디렉션 파일명, 없으면 NULL */
};
```

**파싱 예시:**

| 입력 | `argv[]` | `infile` | `outfile` |
|:-----|:---------|:---------|:----------|
| `ls -l` | `{"ls", "-l", NULL}` | `NULL` | `NULL` |
| `echo hello > out.txt` | `{"echo", "hello", NULL}` | `NULL` | `"out.txt"` |
| `sort < data.txt` | `{"sort", NULL}` | `"data.txt"` | `NULL` |
| `cat < in.txt > out.txt` | `{"cat", NULL}` | `"in.txt"` | `"out.txt"` |

`parse_command()` 함수가 `<`와 `>`를 토큰으로 인식하여 `infile`/`outfile`에 저장하고, 나머지를 `argv[]`에 넣어주는 것이 이미 구현되어 있다.

### 3.3 TODO 1: fork()로 자식 생성

```c
static int execute_command(struct command *cmd)
{
    /* TODO 1: fork()로 자식 프로세스를 생성하세요. */
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    ...
}
```

**왜 fork()가 필요한가?**
- 셸 자체는 계속 실행되어야 한다 (다음 명령어를 받아야 하므로).
- `exec()`는 현재 프로세스의 이미지를 **덮어쓰기** 때문에, 자식에서 실행해야 셸이 살아남는다.
- 이것이 **fork() + exec()** 패턴의 핵심 이유이다.

> **[Lecture — fork() 반환 값]** 강의에서 배운 fork()의 반환 값 규칙이 여기서 그대로 적용된다. 부모는 양수(자식 pid)를 받아 `waitpid()`에 사용하고, 자식은 0을 받아 `exec()` 경로로 분기한다. `-1`은 fork 실패(프로세스 테이블 가득 참 등)를 의미하며, 이를 적절히 처리해야 한다.

### 3.4 TODO 2a: 입력 리디렉션

```c
if (pid == 0) {
    /* TODO 2 (자식 프로세스): */

    /* a. 입력 리다이렉션 */
    if (cmd->infile != NULL) {
        int fd = open(cmd->infile, O_RDONLY);   /* 파일을 읽기 전용으로 열기 */
        if (fd < 0) {
            perror(cmd->infile);
            exit(1);                            /* 파일 열기 실패 시 종료 */
        }
        dup2(fd, STDIN_FILENO);                 /* stdin(fd 0)을 파일로 교체 */
        close(fd);                              /* 원래 fd는 닫기 */
    }
    ...
}
```

**`dup2(fd, STDIN_FILENO)` 동작 원리:**
1. 파일 디스크립터 테이블에서 `STDIN_FILENO`(= 0)이 가리키는 대상을 `fd`와 동일하게 변경
2. 이후 `exec()`로 실행되는 프로그램이 `stdin`에서 읽으면, 실제로는 파일에서 읽게 됨
3. 프로그램은 리디렉션을 **인식하지 못함** — 그냥 stdin에서 읽을 뿐

> **[Lab 실습 4]** `sort < data.txt`를 실행하면, `sort` 프로그램은 평소처럼 `stdin`에서 읽지만, 실제로는 `data.txt`의 내용이 읽힌다. 이것이 UNIX의 I/O 리디렉션이 강력한 이유 — 프로그램을 수정하지 않고도 입출력 대상을 변경할 수 있다.

### 3.5 TODO 2b: 출력 리디렉션

```c
    /* b. 출력 리다이렉션 */
    if (cmd->outfile != NULL) {
        int fd = open(cmd->outfile,
                      O_WRONLY | O_CREAT | O_TRUNC,   /* 쓰기용 | 없으면 생성 | 있으면 덮어쓰기 */
                      0644);                          /* 권한: rw-r--r-- */
        if (fd < 0) {
            perror(cmd->outfile);
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);    /* stdout(fd 1)을 파일로 교체 */
        close(fd);
    }
```

**`open()` 플래그 의미:**

| 플래그 | 의미 |
|:-------|:-----|
| `O_WRONLY` | 쓰기 전용으로 열기 |
| `O_CREAT` | 파일이 없으면 새로 생성 |
| `O_TRUNC` | 파일이 존재하면 내용을 비우고 처음부터 쓰기 (셸의 `>` 동작) |

**권한 `0644`**: 소유자는 읽기+쓰기, 그룹과 기타는 읽기만 가능

### 3.6 TODO 2c, 2d: 명령어 실행

```c
    /* c. 명령어 실행 */
    execvp(cmd->argv[0], cmd->argv);

    /* d. exec 실패 — 이 줄에 도달하면 exec이 실패한 것 */
    perror("minishell");
    exit(127);
}
```

**`execvp()`를 사용하는 이유:**
- `v` (vector): 인자를 `char *argv[]` 배열로 전달 — `struct command`의 `argv`를 그대로 사용 가능
- `p` (path): `PATH` 환경 변수에서 실행 파일을 자동 검색 — 전체 경로(`/bin/ls`)를 쓸 필요 없이 `ls`만으로 충분

**exec 성공 시:**
- 자식 프로세스의 텍스트·데이터·힙·스택이 새 프로그램으로 **완전히 교체**됨
- `execvp()` 이후의 코드는 **절대 실행되지 않음**

**exec 실패 시 (존재하지 않는 명령어 등):**
- `execvp()`가 반환하고 그 아래 코드가 실행됨
- `exit(127)`: 셸 관례상 "command not found"는 종료 코드 127

> **[Lecture — exec() 계열]** 강의에서 배운 exec() 계열 함수(execl, execlp, execle, execv, execvp, execve) 중 `execvp`를 선택한 이유는, 인자가 배열 형태(`argv`)이고 PATH 검색이 필요하기 때문이다. l/v는 인자 전달 방식, p는 PATH 검색 여부, e는 환경 변수 지정 여부를 결정한다.

### 3.7 TODO 3: 부모의 대기

```c
    /* TODO 3 (부모 프로세스): */
    int status;
    waitpid(pid, &status, 0);       /* 특정 자식(pid)의 종료를 대기 */
    if (WIFEXITED(status))
        return WEXITSTATUS(status); /* 정상 종료 시 종료 코드 반환 */
    return -1;                      /* 비정상 종료 */
}
```

**`waitpid()` vs `wait()`:**

| 함수 | 대상 | 사용 시점 |
|:-----|:-----|:---------|
| `wait(NULL)` | 아무 자식이나 하나 | 자식이 1개일 때 간편하게 사용 |
| `waitpid(pid, &status, 0)` | 특정 pid의 자식 | 자식이 여러 개일 때 정확한 제어 |

여기서는 자식이 1개지만, `waitpid()`를 사용하여 **정확히 해당 자식**의 종료 상태를 받아온다.

**종료 상태 매크로:**
- `WIFEXITED(status)` — 자식이 `exit()`으로 정상 종료했는지 확인
- `WEXITSTATUS(status)` — 종료 코드(0~255) 추출

### 3.8 execute_command 전체 흐름 요약

```text
execute_command(cmd) 호출 — 예: "ls -l > out.txt"
    │
    ├── fork()
    │     │
    │     ├── [자식 프로세스]
    │     │     ├── infile이 있으면: open() → dup2(fd, STDIN) → close(fd)
    │     │     ├── outfile이 있으면: open() → dup2(fd, STDOUT) → close(fd)
    │     │     ├── execvp("ls", {"ls", "-l", NULL})
    │     │     │     → 성공: 프로세스 이미지가 ls로 교체됨
    │     │     │     → ls가 stdout(= out.txt)에 출력
    │     │     │     → ls 종료 시 exit()
    │     │     └── exec 실패 시: perror + exit(127)
    │     │
    │     └── [부모 프로세스]
    │           ├── waitpid(pid, &status, 0)  ← 자식 종료까지 블록
    │           └── WEXITSTATUS(status) 반환
    │
    └── 셸의 main()으로 복귀 → 다음 명령어 프롬프트 출력
```

이것이 강의에서 배운 **"fork() + exec() + wait() = 셸의 기본 동작"** 패턴의 구체적 구현이다.

---

<br>

## 4. 과제 2 — minishell.c 해설 (execute_pipe)

### 4.1 파이프가 무엇을 하는가?

셸에서 `cmd1 | cmd2`를 입력하면:
- `cmd1`의 **표준 출력(stdout)**이 파이프를 통해 `cmd2`의 **표준 입력(stdin)**으로 연결된다.

```text
예: ls | wc -l

     ls 프로세스                파이프(커널 버퍼)               wc -l 프로세스
  ━━━━━━━━━━━━━━━            ━━━━━━━━━━━━━━━            ━━━━━━━━━━━━━━━
      stdout ──────────────→ [   커널 버퍼   ] ──────────────→ stdin
   (파일 목록 출력)                                           (줄 수 세기)
```

**execute_pipe()가 해야 할 일:**
1. 파이프 1개 생성 (cmd1 → cmd2 방향)
2. 자식 2개 생성 (cmd1용, cmd2용)
3. 각 자식에서 `dup2()`로 파이프를 stdin/stdout에 연결
4. 부모에서 파이프 닫기 + 자식 2개 모두 `wait()`

> **[Lab 실습 4]** 실습에서 "`ls | wc -l` 구현" 연습 문제가 있었다. 이 과제의 `execute_pipe()`는 바로 그 구현을 일반화한 것으로, 어떤 두 명령어든 파이프로 연결할 수 있게 한다.

### 4.2 TODO 4: 파이프 생성

```c
static int execute_pipe(struct command *cmd1, struct command *cmd2)
{
    /* TODO 4: pipe()로 파이프를 생성하세요. */
    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        return -1;
    }
    ...
}
```

**pingpong과의 차이:**
- pingpong: 양방향 통신 → 파이프 **2개**
- minishell 파이프: 단방향 통신 (cmd1 → cmd2) → 파이프 **1개**

```text
fd[0] = 읽기 끝 → cmd2의 stdin에 연결할 것
fd[1] = 쓰기 끝 → cmd1의 stdout에 연결할 것
```

### 4.3 TODO 5: 자식1 (cmd1 실행)

```c
/* TODO 5: fork()로 자식1을 생성하세요 (cmd1 실행). */
pid_t pid1 = fork();
if (pid1 < 0) {
    perror("fork");
    close(fd[0]);
    close(fd[1]);    /* 오류 시 파이프 정리 */
    return -1;
}

if (pid1 == 0) {
    /* 자식1: cmd1의 stdout을 파이프 쓰기 끝으로 연결 */
    close(fd[0]);                      /* 읽기 끝은 사용 안 함 */
    dup2(fd[1], STDOUT_FILENO);        /* stdout → 파이프 쓰기 끝 */
    close(fd[1]);                      /* dup2 후 원래 fd는 닫기 */

    if (cmd1->infile != NULL) {        /* cmd1에 입력 리디렉션이 있으면 */
        int infd = open(cmd1->infile, O_RDONLY);
        if (infd < 0) { perror(cmd1->infile); exit(1); }
        dup2(infd, STDIN_FILENO);
        close(infd);
    }

    execvp(cmd1->argv[0], cmd1->argv); /* cmd1 실행 */
    perror("minishell");
    exit(127);
}
```

**핵심:** `dup2(fd[1], STDOUT_FILENO)` — cmd1이 `printf()`나 `write(1,...)`로 출력하면, 데이터가 **파이프의 커널 버퍼**로 들어간다.

### 4.4 TODO 6: 자식2 (cmd2 실행)

```c
/* TODO 6: fork()로 자식2를 생성하세요 (cmd2 실행). */
pid_t pid2 = fork();
if (pid2 < 0) {
    perror("fork");
    close(fd[0]);
    close(fd[1]);
    return -1;
}

if (pid2 == 0) {
    /* 자식2: cmd2의 stdin을 파이프 읽기 끝으로 연결 */
    close(fd[1]);                      /* 쓰기 끝은 사용 안 함 */
    dup2(fd[0], STDIN_FILENO);         /* stdin → 파이프 읽기 끝 */
    close(fd[0]);                      /* dup2 후 원래 fd는 닫기 */

    if (cmd2->outfile != NULL) {       /* cmd2에 출력 리디렉션이 있으면 */
        int outfd = open(cmd2->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (outfd < 0) { perror(cmd2->outfile); exit(1); }
        dup2(outfd, STDOUT_FILENO);
        close(outfd);
    }

    execvp(cmd2->argv[0], cmd2->argv); /* cmd2 실행 */
    perror("minishell");
    exit(127);
}
```

**핵심:** `dup2(fd[0], STDIN_FILENO)` — cmd2가 `scanf()`나 `read(0,...)`로 읽으면, 데이터가 **파이프의 커널 버퍼**에서 읽힌다. 즉, cmd1의 출력이 cmd2의 입력이 된다.

### 4.5 TODO 7: 부모에서 정리

```c
/* TODO 7: 부모에서 정리하세요. */
close(fd[0]);                    /* 부모의 파이프 fd 닫기 (매우 중요!) */
close(fd[1]);

int status;
waitpid(pid1, NULL, 0);          /* 자식1(cmd1) 종료 대기 */
waitpid(pid2, &status, 0);       /* 자식2(cmd2) 종료 대기 + 상태 수집 */

if (WIFEXITED(status))
    return WEXITSTATUS(status);  /* cmd2의 종료 코드 반환 (셸 관례) */
return -1;
```

**부모가 파이프의 양쪽을 닫는 것이 왜 중요한가?**

- 부모도 `fork()` 전에 만든 `fd[0]`, `fd[1]`을 갖고 있다.
- 부모가 `fd[1]`(쓰기 끝)을 닫지 않으면:
  - cmd2의 `read()`가 **EOF를 받지 못함** → 영원히 블록됨 → 프로그램이 멈춤!
- UNIX의 파이프 규칙: **모든 쓰기 끝이 닫혀야** 읽기 쪽에서 EOF가 수신됨

> **[Lab 실습 3]** 이것이 실습에서 "쓰기 끝을 닫으면 → 읽기 쪽에서 EOF를 수신"이라고 배운 규칙의 실전 적용이다. 부모, 자식1, 자식2 총 3개의 프로세스가 파이프 fd를 갖고 있으므로, **각 프로세스에서 사용하지 않는 fd를 모두 닫아야** 정상 동작한다.

### 4.6 execute_pipe 전체 흐름 요약

```text
execute_pipe(cmd1, cmd2) 호출 — 예: "ls | wc -l"
    │
    ├── pipe(fd)  → fd[0]=읽기, fd[1]=쓰기
    │
    ├── fork() → 자식1 (cmd1 = "ls")
    │     ├── close(fd[0])                 /* 읽기 끝 안 씀 */
    │     ├── dup2(fd[1], STDOUT_FILENO)   /* stdout → 파이프 */
    │     ├── close(fd[1])
    │     └── execvp("ls", {"ls", NULL})
    │           → ls의 출력이 파이프로 들어감
    │
    ├── fork() → 자식2 (cmd2 = "wc -l")
    │     ├── close(fd[1])                 /* 쓰기 끝 안 씀 */
    │     ├── dup2(fd[0], STDIN_FILENO)    /* stdin ← 파이프 */
    │     ├── close(fd[0])
    │     └── execvp("wc", {"wc", "-l", NULL})
    │           → wc가 파이프에서 읽어 줄 수를 셈
    │
    ├── [부모] close(fd[0]), close(fd[1])  /* 핵심! EOF 전달 보장 */
    ├── waitpid(pid1, ...)
    └── waitpid(pid2, &status, ...)
```

### 4.7 파이프 + 리디렉션 조합

이 구현은 **파이프와 리디렉션의 조합**도 지원한다:

```bash
minishell> echo hello | cat > /tmp/pipe_test.txt
```

이 경우:
- **cmd1** = `echo hello` → stdout이 **파이프**로 연결됨
- **cmd2** = `cat > /tmp/pipe_test.txt` → stdin이 **파이프**에서, stdout이 **파일**로

```text
echo hello                  파이프                    cat
━━━━━━━━                  ━━━━━━                  ━━━
stdout → [파이프] → stdin
                                                stdout → /tmp/pipe_test.txt
```

코드에서 자식2가 `cmd2->outfile`을 확인하여 출력 리디렉션을 추가 설정하는 부분이 이를 처리한다.

---

<br>

## 5. 핵심 개념 정리 및 시험 대비 포인트

### 5.1 시스템 콜 조합 패턴 총정리

| 패턴 | 용도 | 이 과제에서의 적용 |
|:-----|:-----|:-----------------|
| **fork() + wait()** | 자식 생성 후 종료 대기 | pingpong: 부모가 자식을 기다림 |
| **fork() + exec() + wait()** | 새 프로그램 실행 (셸의 기본 동작) | minishell: execute_command() |
| **pipe() + fork() + read/write** | 프로세스 간 데이터 교환 | pingpong: 1바이트 왕복 |
| **pipe() + fork()×2 + dup2() + exec()** | 셸 파이프라인 구현 | minishell: execute_pipe() |
| **open() + dup2() + close()** | I/O 리디렉션 | minishell: 입출력 리디렉션 |

### 5.2 fd 관리 규칙

**규칙 1: 사용하지 않는 파이프 fd는 반드시 닫는다**
- 닫지 않으면 EOF가 전달되지 않아 프로그램이 영원히 블록됨

**규칙 2: `dup2()` 후에는 원래 fd를 닫는다**
- `dup2(fd, STDOUT_FILENO)` 후 `close(fd)` — fd와 STDOUT 두 개가 같은 파일을 가리키므로, 원래 fd는 불필요

**규칙 3: fork() 후에는 3개의 프로세스(부모, 자식1, 자식2)가 모두 파이프 fd를 갖고 있음**
- 각 프로세스에서 불필요한 fd를 닫아야 한다.

```text
fd 소유 상황 (fork() 후):

             fd[0]  fd[1]
       부모:   ✕      ✕     ← 둘 다 닫기
자식1(cmd1):   ✕      ✓     ← fd[0] 닫기, fd[1]을 stdout에 연결 후 닫기
자식2(cmd2):   ✓      ✕     ← fd[1] 닫기, fd[0]을 stdin에 연결 후 닫기
```

### 5.3 이 과제와 강의 내용의 연결

**이론(Lecture) 연계:**

| 강의 개념 | 이 과제에서 직접 체험한 것 |
|:----------|:------------------------|
| 프로세스 = 실행 중인 프로그램 | `fork()` 후 자식 프로세스가 독립적 실행 단위가 됨 |
| 프로세스 생성 (부모→자식) | `fork()`로 자식 생성, `getpid()`/`getppid()`로 관계 확인 |
| fork()의 반환 값 | 부모는 자식 pid, 자식은 0 — 이를 if-else로 분기 |
| exec()로 새 프로그램 적재 | `execvp()`로 자식의 메모리 이미지를 교체 |
| 좀비 프로세스(Zombie) | `wait()`/`waitpid()`를 호출하지 않으면 발생 |
| 문맥 교환(Context Switch) | pingpong의 RTT에 문맥 교환 오버헤드가 포함됨 |
| fork() + exec() + wait() = 셸 | minishell의 execute_command()가 바로 이 패턴 |

**실습(Lab) 연계:**

| 실습 개념 | 이 과제에서 직접 체험한 것 |
|:----------|:------------------------|
| pipe()는 단방향 | pingpong에서 양방향을 위해 2개 사용 |
| 사용 안 하는 fd 닫기 | 모든 프로세스에서 불필요한 fd를 close() |
| dup2()로 I/O 리디렉션 | minishell에서 파일/파이프를 stdin/stdout에 연결 |
| 생산자-소비자 패턴 | 파이프를 통한 cmd1→cmd2 데이터 흐름 |

### 5.4 시험 대비 — 자주 출제되는 포인트

**1. fork() 관련:**
- fork()의 반환 값 (부모: 자식 pid, 자식: 0, 실패: -1)
- fork() 후 부모와 자식의 실행 순서는 **보장되지 않음** (스케줄러 의존)
- fork() 호출 시 부모의 주소 공간이 **복사**됨 (fd 테이블 포함)

**2. exec() 관련:**
- exec() 성공 시 원래 코드로 **반환하지 않음** (메모리가 덮어쓰기되므로)
- exec()은 PID를 변경하지 않음

**3. wait() 관련:**
- wait()을 호출하지 않으면 → **좀비 프로세스**
- 부모가 먼저 종료하면 → **고아 프로세스** → init/systemd에 의해 입양

**4. pipe() 관련:**
- 파이프는 **단방향** — 양방향은 2개 필요
- 모든 쓰기 끝이 닫혀야 읽기 쪽에서 **EOF** 수신
- 사용하지 않는 fd를 닫지 않으면 **데드락(deadlock)** 발생 가능

**5. dup2() 관련:**
- `dup2(oldfd, newfd)` — newfd가 oldfd와 같은 대상을 가리키게 함
- 셸의 `>`, `<`, `|` 모두 dup2()로 구현됨

> **시험 팁:** 위 5개 시스템 콜의 동작 원리를 정확히 이해하고, 코드에서 어떤 순서로 호출해야 하는지(fork 전에 pipe, fork 후에 dup2, exec 전에 리디렉션 설정 등)를 숙지해야 한다.

---

<br>

## 요약

| 개념 | 핵심 정리 |
|:-----|:---------|
| pingpong.c | `pipe()` (양방향 = 2개) + `fork()` + `read()`/`write()` + `wait()`; 파이프를 통한 프로세스 간 동기화와 RTT 측정 |
| fd 관리 | 사용하지 않는 fd는 반드시 close(); dup2() 후 원래 fd 닫기; fork() 후 모든 프로세스에서 불필요한 fd 정리 |
| execute_command() | `fork()` + `exec()` + `wait()` = 셸의 기본 동작 원리; `open()` + `dup2()` + `close()` = I/O 리디렉션 |
| execute_pipe() | `pipe()` + `fork()×2` + `dup2()` = 셸 파이프라인; 부모 포함 3개 프로세스 모두에서 불필요한 fd 닫기 |
| execvp()의 선택 이유 | `v`(배열 인자) + `p`(PATH 검색) — struct command의 argv를 그대로 사용하면서 PATH 자동 검색 |
| 파이프 + 리디렉션 조합 | cmd1의 stdout → 파이프 → cmd2의 stdin; cmd2에 추가로 출력 리디렉션 가능 |
| 시스템 콜 조합 패턴 | fork+wait, fork+exec+wait, pipe+fork+read/write, pipe+fork×2+dup2+exec, open+dup2+close |
| 시험 핵심 | fork 반환 값 구분, exec 성공 시 미반환, wait 미호출 시 좀비, 파이프 단방향 원칙, dup2 동작 원리 |
| 강의 연계 | Lecture: 프로세스 생성/종료/좀비/문맥교환 (Ch 3.1–3.3); Lab: pipe/dup2/fd 관리 (실습 3–4) |

---
