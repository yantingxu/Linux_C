/*
 * 第33章 信号 -std=gun99
 */

// 1. 信号的基本概念
// Ctrl+C => 内核处理 => SIGINT信号记在进程中 => 重新被调度执行前先处理信号
// kill -l 发信号, signal.h
// 默认处理动作: Term(SIGINT), Core(SIGQUIT), Ign, Stop, Cont
// 可选择的处理: 忽略,默认处理,sigaction

// 2. 产生信号
// 2.1 终端按键: SIGINT, SIGQUIT
// 2.2 kill -SIGSEGV 7940
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<stdio.h>

int kill(pid_t pid, int signo);     // 给其它进程发信号
int raise(int signo);               // 给当前进程发信号
// 成功0, 失败-1

#include<stdlib.h>
void abort(void);                   // 给当前进程发SIGABRT信号

// 2.3 软件信号
unsigned int alarm(unsigned int seconds);   // 给当前进程发SIGALRM信号; 返回0或还剩余多少秒; seconds = 0则取消以前设定的闹钟

// 3. 阻塞信号
// 3.1 信号在内核中的表示
// 信号产生 => 信息未决(pending) => 信息阻塞(blocking) => 信号递达(delivery)
// for each handler, there is one bit for blocking, one bit for pending and one entry for handler (all in task_struct)
// 阻塞和未决信号集记录在sigset_t类型的变量中,其中阻塞信号集也称为信号屏蔽字
// 3.2 信号集操作函数
// 只能用这些函数来操作sigset_t的变量

// 初始化
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
// add, delete, check
int sigaddset(sigset_t *set, int signo);
int sigdelset(sigset_t *set, int signo);
int sigismember(const sigset_t *set, int signo);

// 3.3 sigprocmask
int sigprocmask(int how, const sigset_t *set, sigset_t *oset);
// how: SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK
// 如查oset非空则原sigset传出
// 如果解除了未决信号,那么返回前至少一个信号递达

// 3.4 sigpending
int sigpending(sigset_t *set);      // 读取当前未决信号集

void print_sigset(sigset_t *set)
{
    for (int i = 1; i < 32; i++) {
        if (sigismember(set, i) == 1) {
            putchar('1');
        } else {
            putchar('0');
        }
    }
    putchar('\n');
}

void test_sigmask()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);
    while (1) {
        sigset_t p;
        sigpending(&p);
        print_sigset(&p);
        sleep(1);
    }

}

int main(void)
{
    test_sigmask();
    return 0;
}



