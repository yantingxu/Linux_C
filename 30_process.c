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

void test_ps()
{
    execlp("ps", "ps", "aux", NULL);
    perror("exec ps");
    exit(1);
}


int main(void)
{
    // test_environ();
    // test_fork();
    test_ps();
}






