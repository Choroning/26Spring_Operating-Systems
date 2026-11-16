# HW1 과제 안내문 — pingpong과 minishell

[레포지토리 안내](../../../README.ko.md) | [English](Assignment.md) | [한국어](Assignment.ko.md)

> **최종 수정일:** 2026-11-16

운영체제 1차 과제이다. 2주차(프로세스 1)의 프로세스 생성, 파이프, 입출력 리디렉션을 다룬다. 배포 파일인 시작 코드(`skeleton/`)와 자동 테스트 스크립트(`tests/`)는 이 폴더에 원본 그대로 있다.

- **교재:** xv6 Ch 1 - Operating System Interfaces
- **제출 기한:** 다음 수업 시작 전
- **제출물:** 소스 코드 파일(`pingpong.c`, `minishell.c`)

---

<br>

## 개요

2주차 과제는 2개이며, 둘 다 **필수**이다.

| 과제 | 난이도 | 핵심 개념 | 파일 |
|------------|------------|--------------|------|
| 1. pingpong | 기본 | pipe, fork, read/write | `pingpong.c` |
| 2. minishell | 중급 | fork, exec, wait, pipe, dup2 | `minishell.c` |

*표 1. 두 과제의 난이도, 핵심 개념, 파일*

---

<br>

## 환경

```bash
# 필요한 도구
gcc --version   # gcc 필요

# 컴파일 방법 (모든 과제에 공통)
gcc -Wall -o output source.c

# 테스트 실행
cd tests/
chmod +x *.sh
./test_pingpong.sh ../skeleton/pingpong.c
./test_minishell.sh ../skeleton/minishell.c
```

---

<br>

## 과제 1: pingpong (필수)

### 설명

이 과제는 xv6 교재 Ch 1의 Exercise 1을 바탕으로 한다.

두 프로세스(부모와 자식)가 파이프를 통해 1바이트를 주고받는 프로그램을 작성한다.

```
Parent --[ping: 1 byte]--> Child
Parent <--[pong: 1 byte]-- Child
```

### 동작

1. 부모가 자식에게 1바이트(`'p'`)를 보낸다(ping).
2. 자식은 그 바이트를 받고 `<pid>: received ping`을 출력한다.
3. 자식이 부모에게 1바이트를 돌려보낸다(pong).
4. 부모는 그 바이트를 받고 `<pid>: received pong`을 출력한다.
5. 왕복 시간을 측정하여 마이크로초 단위로 출력한다.

### 출력 형식 (이 형식을 정확히 따라야 한다)

```
<childPID>: received ping
<parentPID>: received pong
Round-trip time: X.XXX us
```

예시:

```
12345: received ping
12344: received pong
Round-trip time: 156.000 us
```

### 구현 안내

시작 코드 `skeleton/pingpong.c`에는 TODO 주석이 들어 있다. 각 TODO를 순서대로 구현한다.

**필요한 시스템 콜:** `pipe()`, `fork()`, `read()`, `write()`, `close()`, `wait()`, `getpid()`

**핵심 사항:**

1. 양방향 통신에는 **파이프 2개**가 필요하다.
2. 각 프로세스에서 사용하지 않는 파이프 fd는 `close()`해야 한다.
3. 자식은 ping을 받고, 부모는 pong을 받는다.

**힌트** (막힐 때 참고한다):

<details>
<summary>힌트 1: 파이프 구성</summary>

```
parent_to_child[2]:  부모가 [1]에 쓰고, 자식이 [0]에서 읽는다
child_to_parent[2]:  자식이 [1]에 쓰고, 부모가 [0]에서 읽는다
```

</details>

<details>
<summary>힌트 2: 자식 프로세스가 닫아야 하는 fd</summary>

```c
// 자식은 parent_to_child에서 읽기만 하고 child_to_parent에 쓰기만 한다
close(parent_to_child[1]);  // 쓰기 끝을 닫는다
close(child_to_parent[0]);  // 읽기 끝을 닫는다
```

</details>

<details>
<summary>힌트 3: read와 write의 사용법</summary>

```c
char buf = 'p';
write(parent_to_child[1], &buf, 1);  // 1바이트를 쓴다
read(parent_to_child[0], &buf, 1);   // 1바이트를 읽는다
```

</details>

### 테스트

```bash
cd tests/
./test_pingpong.sh ../skeleton/pingpong.c
```

테스트 항목:
- 컴파일 성공
- 정상 종료
- "received ping"과 "received pong" 출력 확인
- 두 PID가 서로 다름
- 왕복 시간 출력 확인
- 반복 실행에서의 안정성

---

<br>

## 과제 2: minishell (필수)

### 설명

간단한 셸을 구현한다. 이 과제를 통해 실제 셸이 명령어를 실행하는 방식을 이해한다.

### 지원 기능

| 기능 | 예시 | 설명 |
|---------|---------|-------------|
| 단일 명령어 | `ls -l` | fork + execvp + wait |
| 출력 리디렉션 | `echo hello > file.txt` | stdout을 파일로 |
| 입력 리디렉션 | `sort < data.txt` | stdin을 파일에서 |
| 파이프 | `ls \| wc -l` | cmd1의 stdout을 cmd2의 stdin으로 |
| 종료 | `exit` | 셸을 종료한다 |

*표 2. minishell이 지원해야 하는 기능*

### 구현 안내

시작 코드 `skeleton/minishell.c`에는 다음이 이미 구현되어 있다.
- `main()` 함수(입력 읽기, 파이프 분할, exit 처리)
- `parse_command()` 함수(명령어 파싱, 리디렉션 파싱)
- `struct command` 구조체

**학생이 구현해야 하는 함수:**

#### 1. `execute_command(struct command *cmd)`

단일 명령어를 실행한다.

```
[구현 단계]
1. fork()로 자식을 생성한다
2. 자식에서:
   - cmd->infile이 있으면: open() -> dup2(fd, STDIN_FILENO) -> close(fd)
   - cmd->outfile이 있으면: open() -> dup2(fd, STDOUT_FILENO) -> close(fd)
   - execvp(cmd->argv[0], cmd->argv)를 실행한다
   - exec가 실패하면 오류를 출력하고 exit(127)
3. 부모에서: waitpid()로 자식의 종료를 기다린다
```

#### 2. `execute_pipe(struct command *cmd1, struct command *cmd2)`

파이프로 연결된 두 명령어를 실행한다.

```
[구현 단계]
1. pipe(fd)로 파이프를 만든다
2. fork()로 child1을 만든다 (cmd1 실행):
   - fd[0]을 닫는다
   - dup2(fd[1], STDOUT_FILENO)로 stdout을 파이프의 쓰기 끝으로 바꾼다
   - close(fd[1])
   - cmd1->infile이 있으면 입력 리디렉션을 적용한다
   - execvp(cmd1->argv[0], cmd1->argv)
3. fork()로 child2를 만든다 (cmd2 실행):
   - fd[1]을 닫는다
   - dup2(fd[0], STDIN_FILENO)로 stdin을 파이프의 읽기 끝으로 바꾼다
   - close(fd[0])
   - cmd2->outfile이 있으면 출력 리디렉션을 적용한다
   - execvp(cmd2->argv[0], cmd2->argv)
4. 부모에서: fd[0]과 fd[1]을 닫고, 두 자식을 모두 waitpid로 기다린다
```

### execute_command를 먼저 구현한다

`execute_pipe`는 `execute_command`의 확장이다. `execute_command`를 먼저 완성하여 테스트한 뒤 `execute_pipe`를 구현한다.

**힌트** (막힐 때 참고한다):

<details>
<summary>힌트 1: execute_command의 기본 구조</summary>

```c
pid_t pid = fork();
if (pid == 0) {
    // 자식: 리디렉션을 설정한 뒤 exec
    execvp(cmd->argv[0], cmd->argv);
    perror("minishell");
    exit(127);
}
// 부모: 대기
int status;
waitpid(pid, &status, 0);
```

</details>

<details>
<summary>힌트 2: 입력 리디렉션</summary>

```c
if (cmd->infile != NULL) {
    int fd = open(cmd->infile, O_RDONLY);
    if (fd < 0) {
        perror(cmd->infile);
        exit(1);
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
}
```

</details>

<details>
<summary>힌트 3: execute_pipe에서 부모가 해야 하는 일</summary>

부모 프로세스는 파이프의 양쪽 끝을 **반드시** 닫아야 한다. 그렇지 않으면 child2의 `read()`가 EOF를 받지 못해 영원히 블록된다.

```c
close(fd[0]);
close(fd[1]);
waitpid(pid1, NULL, 0);
waitpid(pid2, &status, 0);
```

</details>

### 테스트

```bash
cd tests/
./test_minishell.sh ../skeleton/minishell.c
```

테스트 항목:
- 컴파일 성공
- `echo` 명령어 실행
- 명령어 인자 처리
- 출력 리디렉션(`>`)
- 입력 리디렉션(`<`)
- 파이프(`|`)
- 파이프와 리디렉션의 조합
- `exit` 명령어
- EOF 처리
- 존재하지 않는 명령어의 오류 처리
- 빈 입력 처리
- 명령어의 연속 실행

### 수동 테스트

자동 테스트와 별도로, 셸을 직접 실행하여 여러 명령어를 시험한다.

```bash
$ ./minishell
minishell> ls
minishell> echo hello world
hello world
minishell> echo test > /tmp/test.txt
minishell> cat /tmp/test.txt
test
minishell> cat < /tmp/test.txt
test
minishell> ls | wc -l
minishell> echo hello | cat > /tmp/pipe_test.txt
minishell> cat /tmp/pipe_test.txt
hello
minishell> exit
```

---

<br>

## 제출 방법

다음 파일을 제출한다.
1. `pingpong.c`: 완성한 pingpong 프로그램
2. `minishell.c`: 완성한 minishell 프로그램

### 채점 기준

| 항목 | 배점 |
|------|-------|
| pingpong - 컴파일과 정상 실행 | 10% |
| pingpong - 올바른 출력 형식 | 15% |
| pingpong - 왕복 시간 측정 | 5% |
| minishell - 단일 명령어 실행 | 20% |
| minishell - 출력 리디렉션 | 15% |
| minishell - 입력 리디렉션 | 15% |
| minishell - 파이프 | 20% |

*표 3. 채점 기준*

### 유의 사항

- **모든 코드는 `gcc -Wall`로 경고 없이 컴파일되어야 한다.**
- macOS와 Linux에서 모두 동작해야 한다.
- `fork()` 실패, `exec()` 실패, `open()` 실패 등의 오류 처리를 포함한다.
- 사용하지 않는 파이프 fd는 닫아야 한다(닫지 않으면 프로그램이 멈출 수 있다).
- 이 과제에서는 메모리 누수를 감점하지 않는다.

---

<br>

## 참고 자료

- xv6 교재 Ch 1: Operating System Interfaces (특히 1.1, 1.2, 1.3절)
- `man fork`, `man exec`, `man wait`, `man pipe`, `man dup2`
- 실습 예제 코드: 교수자 저장소의 `week02/2_lab/examples/`

---

<br>

문제를 풀어 본 뒤 [과제 해설](../Assignment-Explanation.ko.md)에서 정답과 풀이를, [코드 총정리](../Code-Walkthrough.ko.md)에서 작성한 코드 전체를 볼 수 있다.
