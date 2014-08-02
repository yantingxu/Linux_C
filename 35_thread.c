/*
 * 第35章 线程
 */

// 1. 线程
// 同一个进程内多个并行的控制流
// 共享: fd, sighandler, working dir, userid/groupid
// 独立: threadid,registers,stack,sigprocmask,errno,priority
// gcc -lpthread

#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>

int pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr, void* (*start_routine)(void*), void *restrict arg);
// restrict关键字: 该指针指向的内存必须通过该指针修改,不能接受其它方式的修改
// thread => threadid
// attr => NULL
// start_routine, arg
// 正常返回0,错误返回错误码(不在errno中,可以用strerror打印)

void printids(const char* msg)
{
    pid_t pid = getpid();
    pthread_t threadid = pthread_self();
    printf("%s pid %u tid %u\n", msg, (unsigned int)pid, (unsigned int)threadid);
}

void* thr_fn(void *arg)
{
    printids(arg);
    return NULL;
}

void test_thread()
{
    pthread_t tid;
    int err = pthread_create(&tid, NULL, thr_fn, "new thread: ");
    if (err != 0) {
        fprintf(stderr, "thread creation fail: %s", strerror(err));
        exit(1);
    }
    printids("main thread: ");
    sleep(3);
}

// 2.2 线止线程
// 1. return (not from main thread, 相当于exit,任何线程exit/_exit都会中止进程内所有的线程)
// 2. pthread_cancel 线止其它线程
// 3. pthread_exit 终止自己

int pthread_cancel(pthread_t tid);
void pthread_exit(void *value_ptr);

int pthread_join(pthread_t tid, void **value_ptr);
// 1. return => returned value
// 2. pthread_cancel => PTHREAD_CANCELED(-1)
// 3. pthread_exit => exit value
// 如果不关心线程的终止状态,那么传value_ptr = NULL
// 如果有返回值return/exit, 那么需要把ptr指向的内存放在heap上
// int pthread_detach(pthread_t tid); 线程置为detach状态,结束立即自己清理,不需要其它线程join
// pthread_join后线程也是detach状态

void* thr_func1()
{
    printf("thread 1 return!\n");
    return (void*)1;
}

void* thr_func2()
{
    printf("thread 2 exit!\n");
    pthread_exit((void*)2);
}

void* thr_func3()
{
    while (1) {
        printf("thread 3 is waiting for cancel!\n");
        sleep(1);
    }
}

void test_join()
{
    pthread_t tid;
    void *ret;

    pthread_create(&tid, NULL, thr_func1, NULL);
    pthread_join(tid, &ret);
    printf("thread 1 exit code: %d\n", (int)ret);

    pthread_create(&tid, NULL, thr_func2, NULL);
    pthread_join(tid, &ret);
    printf("thread 2 exit code: %d\n", (int)ret);

    pthread_create(&tid, NULL, thr_func3, NULL);
    sleep(3);
    pthread_cancel(tid);
    pthread_join(tid, &ret);
    printf("thread 3 exit code: %d\n", (int)ret);
}

// 3. 线程间同步
// 3.1 Mutex (通过汇编swap/xchange实现)
// Mutex上有一个等待队列;
// 同一个线程两次lock, 或者两个线程各持有一个lock => 死锁 => 资源编号,每个线程按照编号lock; 或者使用trylock代替lock
int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;      // 全局或静态变量mutex的初始化

int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);      // 不会阻塞,而是返回EBUSY
int pthread_mutex_unlock(pthread_mutex_t *mutex);
// 成功返回0,失败返回错误号

#define NLOOP 5000
int counter;
void* doit(void* vptr) {
    int i, val;
    for (i = 0; i < NLOOP; i++) {
        pthread_mutex_lock(&counter_mutex);
        counter = counter + 1;
        printf("%u: %d\n", (unsigned int)pthread_self(), counter);
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

void test_mutex()
{
    pthread_t tidA, tidB;
    pthread_create(&tidA, NULL, doit, NULL);
    pthread_create(&tidB, NULL, doit, NULL);
    pthread_join(tidA, NULL);
    pthread_join(tidB, NULL);
}

// 3.2 Conditional Variable
// 阻塞等待一个条件成立, 与一个mutex同时使用

int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);
int pthread_cond_destroy(pthread_cond_t *cond);
pthread_cond_t has_product = PTHREAD_COND_INITIALIZER;

int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
int pthread_cond_timewait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex, const struct timespec *restrict abstime);     // ETIMEOUT
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);

struct msg {
    struct msg* next;
    int num;
};

struct msg* head = NULL;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* consumer(void* p)
{
    struct msg* mp;
    while (1) {
        pthread_mutex_lock(&lock);
        while (head == NULL)
            pthread_cond_wait(&has_product, &lock);
        mp = head;
        head = mp->next;
        pthread_mutex_unlock(&lock);

        printf("Consume: %d\n", mp->num);
        free(mp);
        sleep(3);
    }
}

void* producer(void* p)
{
    struct msg* mp;
    while (1) {
        mp = (struct msg*)malloc(sizeof(struct msg));
        mp->num = rand() % 1000 + 1;
        printf("Produce %d\n", mp->num);

        pthread_mutex_lock(&lock);
        mp->next = head;
        head = mp;
        pthread_cond_signal(&has_product);
        pthread_mutex_unlock(&lock);

        sleep(3);
    }
}

void test_cond()
{
    pthread_t pid, cid;
    pthread_create(&pid, NULL, producer, NULL);
    pthread_create(&cid, NULL, consumer, NULL);
    pthread_join(pid, NULL);
    pthread_join(cid, NULL);
}


int main(void)
{
    // test_thread();
    // test_join();
    // test_mutex();
    test_cond();
    return 0;
}









