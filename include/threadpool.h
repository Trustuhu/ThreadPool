#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

//线程池头文件

#include"condition.h"

//封装线程池中对象所需要执行的任务对象
typedef struct tast
{
	//函数指针，需要执行的任务
	void *(*run)(void * args);
	//参数
	void *args;
	//任务队列中下一个任务
	struct task *next;
}task_t;

//线程池结构体
typedef struct threadpool
{
	//状态量
	condition_t ready;
	//任务队列中第一个任务
	task_t *first;
	//任务队列中最后一个任务
	task_t *last;
	//线程池中已有线程数
	int counter;
	//线程池中空闲线程数
	int idle;
	//线程池中最大线程数
	int max_threads;
	//是否退出标志
	int quit;
}threadpool_t;

//创建的线程执行
void *thread_routine(void *arg);
//线程池初始化
void threadpool_init(threadpool_t *pool, int thread);
//往线程池中加任务
void threadpool_add_task(threadpool_t *pool, void *(*run)(void *args), void *args);
//摧毁线程池
void threadpool_destroy(threadpool_t *pool);

#endif
