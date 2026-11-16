# HW2 Code Walkthrough — Adding the trace System Call

[Repository Guide](../../README.md) | [English](Code-Walkthrough.md) | [한국어](Code-Walkthrough.ko.md)

> **Last Updated:** 2026-11-16

> **Learning Objectives:**
> 1. Read the files, change locations, added lines, and unchanged lines in a patch in unified diff format
> 2. Tell whether each change in the patch was already in the distributed patch and whether it belongs to the required or the bonus task
> 3. Explain line by line how `syscall()` finds the handler and name by the system call number and prints the trace line
> 4. Explain how `freemem_count()`, `proc_count()`, and `sys_sysinfo()` use locks and `copyout()`

---

## Table of Contents

- [1. File Layout](#1-file-layout)
- [2. How to Read the Patch](#2-how-to-read-the-patch)
- [3. Build Settings and Prototypes](#3-build-settings-and-prototypes)
- [4. kernel/kalloc.c](#4-kernelkallocc)
- [5. kernel/proc.c and kernel/proc.h](#5-kernelprocc-and-kernelproch)
- [6. kernel/syscall.c and kernel/syscall.h](#6-kernelsyscallc-and-kernelsyscallh)
- [7. kernel/sysinfo.h and kernel/sysproc.c](#7-kernelsysinfoh-and-kernelsysprocc)
- [8. User-Side Files](#8-user-side-files)
- [Summary](#summary)
- [Self-Check Questions](#self-check-questions)

---

<br>

## 1. File Layout

> **Try It First:** This document contains the complete code and line-by-line explanations. Solve the problems in the [assignment handout](Original/Assignment.md) yourself before reading on.

The companion document [HW2 Assignment Explanation](Assignment-Explanation.md) summarizes the requirements, the system call path, the implementation steps, and the test results. This document reads the submitted `Solution/trace.patch` (450 lines) from beginning to end in file order. Since the patch was made with `git diff` in the xv6 working tree, the files appear in alphabetical order of their paths.

| Patch lines | File | Change | Relation to the distributed patch |
|:-----|:-----|:-----|:-----|
| 1-12 | `Makefile` | Add `_trace_test` to `UPROGS` | Added (optional item in the handout) |
| 13-32 | `kernel/defs.h` | Prototypes of `freemem_count()` and `proc_count()` | Added (bonus) |
| 33-58 | `kernel/kalloc.c` | `freemem_count()` | Added (bonus) |
| 59-103 | `kernel/proc.c` | Reset in `freeproc()`, copy in `kfork()`, `proc_count()` | TODOs written; `proc_count()` added (bonus) |
| 104-113 | `kernel/proc.h` | `trace_mask` field | Unchanged |
| 114-174 | `kernel/syscall.c` | Prototypes, table entries, `syscall_names[]`, trace output | `trace` prototype and entry unchanged, the rest written, `sysinfo` entries added |
| 175-184 | `kernel/syscall.h` | `SYS_trace`, `SYS_sysinfo` | `SYS_trace` unchanged, `SYS_sysinfo` added |
| 185-194 | `kernel/sysinfo.h` | `struct sysinfo` (new file) | Added (bonus) |
| 195-239 | `kernel/sysproc.c` | Include, `sys_trace()`, `sys_sysinfo()` | `sys_trace()` body written, the rest added (bonus) |
| 240-426 | `user/trace_test.c` | Test program (new file) | Same as the distributed `Original/trace_test.c` |
| 427-440 | `user/user.h` | Declarations of `trace()` and `sysinfo()` | `trace()` unchanged, `sysinfo()` added |
| 441-450 | `user/usys.pl` | Entries for `trace` and `sysinfo` | `trace` unchanged, `sysinfo` added |

*Table 1. Per-file structure of the patch and its relation to the distributed patch*

In the table, "unchanged" means that the same line was already in the distributed `Original/trace.patch`, and "written" means that a TODO comment in the distributed patch was filled in.

---

<br>

## 2. How to Read the Patch

The patch is in unified diff format. A `diff --git a/path b/path` line starts one file, the `index` line gives the content hashes before and after the change and the file mode, and the `---` and `+++` lines give the file names before and after. A line such as `@@ -169,6 +169,7 @@ freeproc(struct proc *p)` is the header of a change unit (hunk): 6 lines starting at line 169 of the original file became 7 lines starting at line 169 of the new file, and the trailing text is the name of the function around that location. In the body, lines starting with `+` are added lines, and lines starting with a space are unchanged surrounding context. This patch has no removed lines (`-`).

The hashes in the `index` lines, such as `22a5401` (proc.c) and `076d965` (syscall.c), match those of the distributed patch, which shows that both patches were made against the same xv6 commit.

---

<br>

## 3. Build Settings and Prototypes

```diff
diff --git a/Makefile b/Makefile
index b262c0a..2c89047 100755
--- a/Makefile
+++ b/Makefile
@@ -145,6 +145,7 @@ UPROGS=\
 	$U/_logstress\
 	$U/_forphan\
 	$U/_dorphan\
+	$U/_trace_test\
 
 fs.img: mkfs/mkfs README $(UPROGS)
 	mkfs/mkfs fs.img README $(UPROGS)
diff --git a/kernel/defs.h b/kernel/defs.h
index 122d9ca..2231b69 100755
--- a/kernel/defs.h
+++ b/kernel/defs.h
@@ -59,6 +59,7 @@ void            ireclaim(int);
 void*           kalloc(void);
 void            kfree(void *);
 void            kinit(void);
+uint64          freemem_count(void);
 
 // log.c
 void            initlog(int, struct superblock*);
@@ -101,6 +102,7 @@ void            yield(void);
 int             either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
 int             either_copyin(void *dst, int user_src, uint64 src, uint64 len);
 void            procdump(void);
+uint64          proc_count(void);
 
 // swtch.S
 void            swtch(struct context*, struct context*);
```

Line 9 adds `$U/_trace_test` to `UPROGS` in the `Makefile`. Programs in `UPROGS` are put into the file system image `fs.img` at build time, so it can be run as `trace_test` in the xv6 shell. The handout lists this change as optional.

Lines 21 and 29 add the prototypes of the two helper functions of the bonus task to the `kalloc.c` and `proc.c` sections of `kernel/defs.h`, respectively. Since `defs.h` is the shared declaration header included by every `.c` file of the kernel, placing the prototypes here lets `sysproc.c` call the two functions.

---

<br>

## 4. kernel/kalloc.c

```diff
diff --git a/kernel/kalloc.c b/kernel/kalloc.c
index 0699e7e..7bd93e3 100755
--- a/kernel/kalloc.c
+++ b/kernel/kalloc.c
@@ -80,3 +80,21 @@ kalloc(void)
     memset((char*)r, 5, PGSIZE); // fill with junk
   return (void*)r;
 }
+
+// Count the number of bytes of free memory
+uint64
+freemem_count(void)
+{
+  struct run *r;
+  uint64 count = 0;
+
+  acquire(&kmem.lock);
+  r = kmem.freelist;
+  while(r) {
+    count++;
+    r = r->next;
+  }
+  release(&kmem.lock);
+
+  return count * PGSIZE;
+}
```

`freemem_count()` goes at the end of the file, right after `kalloc()`. `struct run` on line 46 is the structure of `kalloc.c` that uses the first part of a free page as a linked list node. Line 49 takes `kmem.lock`, lines 50-54 follow `next` from `kmem.freelist` counting the nodes, and line 55 releases the lock. Line 57 returns the page count times `PGSIZE` as a number of bytes. The result is a `uint64` to match the field type of `struct sysinfo`.

---

<br>

## 5. kernel/proc.c and kernel/proc.h

```diff
diff --git a/kernel/proc.c b/kernel/proc.c
index 22a5401..9d62297 100755
--- a/kernel/proc.c
+++ b/kernel/proc.c
@@ -169,6 +169,7 @@ freeproc(struct proc *p)
   p->killed = 0;
   p->xstate = 0;
   p->state = UNUSED;
+  p->trace_mask = 0;
 }
 
 // Create a user page table for a given process, with no user memory,
@@ -282,6 +283,9 @@ kfork(void)
   // Cause fork to return 0 in the child.
   np->trapframe->a0 = 0;
 
+  // Copy trace_mask from parent to child
+  np->trace_mask = p->trace_mask;
+
   // increment reference counts on open file descriptors.
   for(i = 0; i < NOFILE; i++)
     if(p->ofile[i])
```

The first hunk adds `p->trace_mask = 0;` (line 67) at the end of `freeproc()`, right after `p->state = UNUSED;`. This is the place of TODO 7 in the distributed patch. The second hunk copies the mask in `kfork()` after setting the child's a0 to 0 and before incrementing the reference counts of the open files (lines 75-76). The comment on line 75 is the distributed patch's TODO comment without `TODO:`. At this point `p` is `myproc()`, that is, the parent that called `fork()`, and `np` is the child set up by `allocproc()`.

```diff
@@ -659,6 +663,22 @@ either_copyin(void *dst, int user_src, uint64 src, uint64 len)
   }
 }
 
+// Count the number of active processes (state != UNUSED)
+uint64
+proc_count(void)
+{
+  struct proc *p;
+  uint64 count = 0;
+
+  for(p = proc; p < &proc[NPROC]; p++) {
+    acquire(&p->lock);
+    if(p->state != UNUSED)
+      count++;
+    release(&p->lock);
+  }
+  return count;
+}
+
 // Print a process listing to console.  For debugging.
 // Runs when user types ^P on console.
 // No lock to avoid wedging a stuck machine further.
diff --git a/kernel/proc.h b/kernel/proc.h
index d021857..60559ee 100755
--- a/kernel/proc.h
+++ b/kernel/proc.h
@@ -104,4 +104,5 @@ struct proc {
   struct file *ofile[NOFILE];  // Open files
   struct inode *cwd;           // Current directory
   char name[16];               // Process name (debugging)
+  int trace_mask;              // Trace syscall mask
 };
```

The third hunk adds `proc_count()` after `either_copyin()` (lines 85-99). The loop on line 92 points to every slot of the global array `proc` in turn, and lines 93-96 take each slot's lock, read `state`, and release it immediately. Since only one lock is held at a time, the state of other slots can change during the count, so the result is an approximation over the moments of the traversal. That is sufficient for reporting the number of processes in use.

The change in `kernel/proc.h` (line 112) adds `int trace_mask` as the last field of `struct proc`, the same as in the distributed patch.

---

<br>

## 6. kernel/syscall.c and kernel/syscall.h

```diff
diff --git a/kernel/syscall.c b/kernel/syscall.c
index 076d965..fca98f1 100755
--- a/kernel/syscall.c
+++ b/kernel/syscall.c
@@ -101,6 +101,8 @@ extern uint64 sys_unlink(void);
 extern uint64 sys_link(void);
 extern uint64 sys_mkdir(void);
 extern uint64 sys_close(void);
+extern uint64 sys_trace(void);
+extern uint64 sys_sysinfo(void);
 
 // An array mapping syscall numbers from syscall.h
 // to the function that handles the system call.
@@ -126,6 +128,35 @@ static uint64 (*syscalls[])(void) = {
 [SYS_link]    sys_link,
 [SYS_mkdir]   sys_mkdir,
 [SYS_close]   sys_close,
+[SYS_trace]   sys_trace,
+[SYS_sysinfo] sys_sysinfo,
+};
+
+// Syscall name string array (for trace output)
+static char *syscall_names[] = {
+[SYS_fork]    "fork",
+[SYS_exit]    "exit",
+[SYS_wait]    "wait",
+[SYS_pipe]    "pipe",
+[SYS_read]    "read",
+[SYS_kill]    "kill",
+[SYS_exec]    "exec",
+[SYS_fstat]   "fstat",
+[SYS_chdir]   "chdir",
+[SYS_dup]     "dup",
+[SYS_getpid]  "getpid",
+[SYS_sbrk]    "sbrk",
+[SYS_pause]   "pause",
+[SYS_uptime]  "uptime",
+[SYS_open]    "open",
+[SYS_write]   "write",
+[SYS_mknod]   "mknod",
+[SYS_unlink]  "unlink",
+[SYS_link]    "link",
+[SYS_mkdir]   "mkdir",
+[SYS_close]   "close",
+[SYS_trace]   "trace",
+[SYS_sysinfo] "sysinfo",
 };
```

Lines 122-123 are the prototypes of the two handlers, declared `extern` because they are defined in `sysproc.c`. Lines 131-132 add two entries to `syscalls[]`, and the `};` on line 133 closes the array. Lines 135-160 are `syscall_names[]`, written in place of the distributed patch's TODO, holding the names of the existing 21 system calls and the new `trace` and `sysinfo` in number order. The `};` on line 160 is the line that originally closed `syscalls[]` and now closes this array. The names are string constants, so they are placed in the kernel's read-only data.

```diff
 
 void
@@ -139,6 +170,11 @@ syscall(void)
     // Use num to lookup the system call function for num, call it,
     // and store its return value in p->trapframe->a0
     p->trapframe->a0 = syscalls[num]();
+
+    // Trace: if the bit for this syscall is set, print trace info
+    if(p->trace_mask & (1 << num)) {
+      printf("%d: syscall %s -> %d\n", p->pid, syscall_names[num], (int)p->trapframe->a0);
+    }
   } else {
     printf("%d %s: unknown sys call %d\n",
             p->pid, p->name, num);
```

`syscall()` reads the number from a7 of the trapframe and calls the handler only if the number is at least 1, within the array bounds, and registered. The patch puts the trace check right after the line that stores the return value in a0 (lines 167-171). The condition on line 169 checks whether the mask, with only bit `num` kept, is nonzero. Line 170 prints exactly the handout's format with `p->pid`, the string from the name array, and the return value cast to `(int)`. The kernel's `printf()` writes directly to the console, so its output can appear interleaved with the output of user programs.

```diff
diff --git a/kernel/syscall.h b/kernel/syscall.h
index 3dd926d..65d8104 100755
--- a/kernel/syscall.h
+++ b/kernel/syscall.h
@@ -20,3 +20,5 @@
 #define SYS_link   19
 #define SYS_mkdir  20
 #define SYS_close  21
+#define SYS_trace  22
+#define SYS_sysinfo 23
```

`kernel/syscall.h` defines `SYS_trace` as 22 and `SYS_sysinfo` as 23. Both the kernel and user programs include this header, so both sides use the same numbers.

---

<br>

## 7. kernel/sysinfo.h and kernel/sysproc.c

```diff
diff --git a/kernel/sysinfo.h b/kernel/sysinfo.h
new file mode 100644
index 0000000..d7eab3f
--- /dev/null
+++ b/kernel/sysinfo.h
@@ -0,0 +1,4 @@
+struct sysinfo {
+  uint64 freemem;   // Available memory (bytes)
+  uint64 nproc;     // Number of active processes (state != UNUSED)
+};
```

The new file `kernel/sysinfo.h` defines only `struct sysinfo`. Since `uint64` is a type from `kernel/types.h`, a file including this header must include `types.h` before it. The `new file mode 100644` line indicates that this file is newly created.

```diff
diff --git a/kernel/sysproc.c b/kernel/sysproc.c
index 419e727..77c0f20 100755
--- a/kernel/sysproc.c
+++ b/kernel/sysproc.c
@@ -6,6 +6,7 @@
 #include "spinlock.h"
 #include "proc.h"
 #include "vm.h"
+#include "sysinfo.h"
 
 uint64
 sys_exit(void)
@@ -107,3 +108,32 @@ sys_uptime(void)
   release(&tickslock);
   return xticks;
 }
+
+// sys_trace: set the trace mask for the current process
+uint64
+sys_trace(void)
+{
+  int mask;
+
+  argint(0, &mask);
+  myproc()->trace_mask = mask;
+  return 0;
+}
+
+// sys_sysinfo: collect system information and copy to user space
+uint64
+sys_sysinfo(void)
+{
+  uint64 addr;
+  struct sysinfo info;
+  struct proc *p = myproc();
+
+  argaddr(0, &addr);
+
+  info.freemem = freemem_count();
+  info.nproc = proc_count();
+
+  if(copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
+    return -1;
+  return 0;
+}
```

Line 203 includes `sysinfo.h` in `sysproc.c`. `sys_trace()` on lines 212-221 turns the three steps of the distributed patch's TODO comment directly into code: it reads the first argument into the local variable `mask`, stores it in the calling process's `trace_mask`, and returns 0.

`sys_sysinfo()` on lines 223-239 reads the address of the user buffer into `addr` (line 231), fills the local variable `info` on the kernel stack with the two helper functions (lines 233-234), and copies `sizeof(info)`, that is, 16 bytes, with `copyout()` through the calling process's page table `p->pagetable` (line 236). It returns -1 if the copy fails and 0 on success.

---

<br>

## 8. User-Side Files

Lines 240-426 add `user/trace_test.c` as a new file. Its content is the same as the 181 lines of the distributed `Original/trace_test.c`, and it runs five tests in turn. Test 1 traces `fork`, Test 2 traces `read` and `write` through a pipe, Test 3 traces `getpid`, `fork`, and `write` together, Test 4 checks that the `write` of a child forked after enabling the mask is traced, and Test 5 checks that nothing is printed after `trace(0)`. The results and the features of the output are in Section 6 of the [assignment explanation](Assignment-Explanation.md).

```diff
diff --git a/user/user.h b/user/user.h
index ac84de9..6d5fdb4 100755
--- a/user/user.h
+++ b/user/user.h
@@ -24,6 +24,9 @@ int getpid(void);
 char* sys_sbrk(int,int);
 int pause(int);
 int uptime(void);
+int trace(int);
+struct sysinfo;
+int sysinfo(struct sysinfo*);
 
 // ulib.c
 int stat(const char*, struct stat*);
diff --git a/user/usys.pl b/user/usys.pl
index c5d4c3a..08df9f2 100755
--- a/user/usys.pl
+++ b/user/usys.pl
@@ -42,3 +42,5 @@ entry("getpid");
 entry("sbrk");
 entry("pause");
 entry("uptime");
+entry("trace");
+entry("sysinfo");
```

Line 435 of `user/user.h` is the prototype of `trace()`. `struct sysinfo;` on line 436 is a forward declaration that announces only the name of the structure without its contents, allowing the prototype on line 437 to take a pointer to that structure. A user program must include `kernel/sysinfo.h` separately to access the fields. Lines 449-450 of `user/usys.pl` generate the stubs for `trace` and `sysinfo` in `usys.S` at build time. Each stub puts the number in a7, as in `li a7, SYS_trace`, and executes `ecall` and `ret`.

---

<br>

## Summary

| Concept | Key Summary |
|:-----|:-----|
| Patch format | `diff --git` marks a file and the `@@` header a change location; `+` lines are added lines and lines starting with a space are context. |
| Relation to the distributed patch | The number, stub, declaration, field, and the prototype and table entry of `trace` were given; the body of `sys_trace()`, the name array, the trace output, the inheritance, and the reset were written; and all changes for `sysinfo` were added. |
| Tracing in syscall() | Right after storing the handler's return value in a0, it tests `trace_mask & (1 << num)` and prints the handout's format using the name array and an `(int)` cast. |
| Helper functions | `freemem_count()` counts the free list while holding `kmem.lock`, and `proc_count()` counts processes that are not `UNUSED` while taking each slot's lock. |
| sys_sysinfo | It fills a structure on the kernel stack and copies 16 bytes through the user page table with `copyout()`, returning -1 on failure. |
| User side | The entries in `usys.pl` generate stubs that put the number in a7 and execute `ecall`, and a forward declaration in `user.h` lets `sysinfo()` take a structure pointer. |

---

<br>

## Self-Check Questions

1. **Hunk header:** In `@@ -282,6 +283,9 @@ kfork(void)`, state what each pair of numbers means and why the new file has 3 more lines.

   <details>
   <summary>Answer</summary>

   `-282,6` means 6 lines starting at line 282 of the original file, and `+283,9` means 9 lines starting at line 283 of the new file. The starting line moved by 1 because one line was added earlier, in `freeproc()`. This hunk adds 3 lines, a comment, an assignment, and a blank line, so 6 lines become 9.

   </details>

2. **Indices of the name array:** State what goes wrong if `syscall_names[]` simply lists the strings without designated indices such as `[SYS_fork] "fork"`.

   <details>
   <summary>Answer</summary>

   Without designated indices, the first string goes into slot 0, so the names are off by one from the system call numbers, which start at 1. For example, `syscall_names[SYS_fork]`, slot 1, would be the second string, `"exit"`. With designated indices, the number-to-name mapping is fixed regardless of the listing order.

   </details>

3. **Header order:** State what happens if `#include "sysinfo.h"` is placed before `#include "types.h"` in `sysproc.c`.

   <details>
   <summary>Answer</summary>

   The field type `uint64` in `sysinfo.h` is defined by `types.h`, so including it before that header leaves the compiler unaware of `uint64` and causes a compilation error. The patch avoids this by including it after all the existing headers.

   </details>
