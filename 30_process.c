/*
 * 第30章 进程
 */

// 1. 引言
// PCB: task_struct in kernel; copied from parent process (the same umask)
// process_id: pid_t类型
// fork + exec

// 2. 环境变量
// when exec is called, cmd line params and environs are passed to main function
// address space: (from high to low) cmd+environs => stack => heap => bss => data => text; the last two parts are read from object file, bss is uninited
// environs: char **, end with NULL

#include<stdio.h>
#include<stdlib.h>

char *genenv(const char*);      // NULL if not found
int setenv(const char*, const char*, int rewrite);  // rewrite != 0 then overlap; otherwise do nothing; return non-zero if error occurs
void unsetenv(const char*);

int test_environ(void)
{
    extern char **environ;
    for (int i = 0; environ[i] != NULL; i++) {
        printf("%s\n", environ[i]);
    }
    printf("========== %s ===========\n", getenv("PATH"));
    setenv("PATH", "TEST", 1);
    printf("========== %s ===========\n", getenv("PATH"));
    unsetenv("PATH");
    return 0;
}

// 3. 进程控制
// Shell only waits for the parent process, but not for the child
// fork后file结构体引用计数增加
// gdb: set follow-fork-mode child
#include<sys/types.h>
#include<unistd.h>

pid_t fork(void);
pid_t getpid(void);
pid_t getppid(void);

#define BUFFSIZE 50
int test_fork(void)
{
    char* message = (char*)malloc(BUFFSIZE);
    int n;
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed!");
        exit(1);
    }
    if (pid == 0) {
        sprintf(message, "%d-%d-%s\n", (int)getppid(), (int)getpid(), "This is child!");
        n = 6;
    } else {
        sprintf(message, "%d-%d-%s\n", (int)getppid(), (int)getpid(), "This is parent!");
        n = 3;
    }
    for (int i = 0; i < n; i++) {
        printf("%s", message);
        sleep(1);
    }
    return 0;
}

// exec: address is replaced by new program; 出错返回-1, 成功不返回
// 带不带p: path(const char* path) ; otherwise treat as a file and search in PATH(const char* file)
// l, v: list (const char* arg, ..., NULL), v(char* const argv[], end with NULL)
// e: char* const envp[]
// 最终都是调用execve, 其它都是转化这种形式
// exec后各个fd还是打开的, 可以dup2做重定向

void test_ps()
{
    execlp("ps", "ps", "aux", NULL);
    perror("exec ps");
    exit(1);
}

// wait and waitpid: 子进程结束后,PCB未释放; 这个两个函数取得退出状态并彻底清除这个进程
// 没有被wait的进程称为僵局进程(不能被kill); 如果父进程结束了,那么子进程的父进程变为init
// 成功返回子进程pid, 出错返回-1;
// 可能会阻塞,返回,出错返回

#include<sys/wait.h>

pid_t wait(int* status);        // status可以是空指针
pid_t waitpid(pid_t pid, int* status, int options);

void test_wait()
{
    pid_t pid = fork();
    if (pid == 0) {
        for (int i = 0; i < 3; i++) {
            printf("This is a child process!\n");
            sleep(1);
        }
        exit(3);
    } else {
        int stat_val;
        waitpid(pid, &stat_val, 0);
        if (WIFEXITED(stat_val)) {
            printf("Child Exit Code: %d\n", WEXITSTATUS(stat_val));
        } else if (WIFSIGNALED(stat_val)) {
            printf("Child Signal Code: %d\n", WTERMSIG(stat_val));
        }
    }
}

// 4. 进程间通信
// 进程间交换数据必须通过内核,在内核中开辟一空缓冲区
// 4.1 管道: 只能单向通信,通过fork继承而来,read/write阻塞
// 4.2 其它机制: fork, exit/wait, file, signal, pipe, fifo, mmap, unix domain socket

#include<unistd.h>
int pipe(int fd[2]);       // fd[0]为读端,fd[1]为写端; 成功返回0, 失败-1

#define MAXLINE 80
void test_pipe()
{
    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe");
        exit(1);
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    if (pid == 0) {
        close(fd[1]);
        char line[MAXLINE];
        int n = read(fd[0], line, MAXLINE);
        write(STDOUT_FILENO, line, n);
    } else {
        close(fd[0]);
        write(fd[1], "hello world\n", 12);
        wait(NULL);
    }
}

int main(void)
{
    // test_environ();
    // test_fork();
    // test_ps();
    // test_wait();
    test_pipe();
}





