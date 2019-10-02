#include"threadpool.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<time.h>



//创建的线程执行
void *thread_routine(void *arg)
{
	struct timespec abstime;
	int timeout;
	printf("thread %d is starting\n", (int)pthread_self());
	threadpool_t *pool=(threadpool_t *)arg;
	while(1)
	{
		timeout=0;
		//访问线程池之前需加锁
		condition_lock(&pool->ready);
		//空闲
		pool->idle++;
		//等待队列有任务到来 或 收到线程池销毁通知
		while(pool->first==NULL && !pool->quit)
		{
			//否则线程阻塞等待
			printf("thread %d is waiting\n", (int)pthread_self());
			//获取当前时间，并加上等待时间，设置进程的超时睡眠时间
			clock_gettime(CLOCK_REALTIME, &abstime);
			abstime.tv_sec+=2;
			int status;
			//该函数会解锁，允许其他线程访问，当被唤醒时，加锁
			status=condition_timedwait(&pool->ready, &abstime);
			if(status==ETIMEDOUT)
			{
				printf("thread %d wait time out\n", (int)pthread_self());
				timeout=1;
				break;
			}
		}
		pool->idle--;
		if(pool->first!=NULL)
		{
			//取出等待队列最前的任务
			task_t *t=pool->first;
			pool->first=t->next;
			//由于任务执行需要消耗时间，先解锁让其他线程访问线程池
			condition_unlock(&pool->ready);
			//执行任务
			t->run(t->args);
			//执行完任务释放内存
			free(t);
			//重新加锁
			condition_lock(&pool->ready);
		}
		
		//退出线程池
		if(pool->quit && pool->first==NULL)
		{
			//当前工作的线程数减一
			pool->counter--;
			//若线程池中没有线程，通知等待线程（主线程）全部任务已完成
			if(pool->counter==0)
			{
				condition_signal(&pool->ready);		
			}
			condition_unlock(&pool->ready);
			break;
		}
		
		//超时，跳出销毁线程
		if(timeout==1)
		{
			//当前工作的线程数减一
			pool->counter--;
			condition_unlock(&pool->ready);
			break;
		}

		condition_unlock(&pool->ready);		
	}

	printf("thread %d is exiting\n", (int)pthread_self());
	return NULL;
}

//线程池初始化
void threadpool_init(threadpool_t *pool, int threads)
{
	condition_init(&pool->ready);
	pool->first=NULL;
	pool->last=NULL;
	pool->counter=0;
	pool->idle=0;
	pool->max_threads=threads;
	pool->quit=0;
}

//往线程池中加任务
void threadpool_add_task(threadpool_t *pool, void *(*run)(void *arg), void *arg)
{
	//产生一个新的任务
	task_t *newtask=(task_t *)malloc(sizeof(task_t));
	newtask->run=run;
	newtask->args=arg;
	//新加的任务放入队列尾端
	newtask->next=NULL;

	//线程池的状态被多个线程共享，操作前需加锁
	condition_lock(&pool->ready);

	//第一个任务加入
	if(pool->first==NULL)
	{
		pool->first=newtask;
	}
	else
	{
		pool->last->next=newtask;
	}
	//从队列尾指向新加入的线程
	pool->last=newtask;

	//线程池中有空闲线程
	if(pool->idle>0)
	{
		condition_signal(&pool->ready);
	}
	//当前线程池中线程个数没有达到设定的最大值
	else if(pool->counter<pool->max_threads)
	{
		pthread_t tid;
		pthread_create(&tid, NULL, thread_routine, pool);
		pool->counter++;
	}
	//结束，访问
	condition_unlock(&pool->ready);
}

//摧毁线程池
void threadpool_destroy(threadpool_t *pool)
{
	//如果已经调用销毁，直接返回
	if(pool->quit)
	{
		return;
	}
	//加锁
	condition_lock(&pool->ready);
	//设置销毁标记为1
	pool->quit=1;
	//线程池中线程数大于0
	if(pool->counter>0)
	{
		//对于等待的线程，发送信号唤醒
		if(pool->idle>0)
		{
			condition_broadcast(&pool->ready);
		}
		//正在执行任务的线程，等待他们结束任务
		while(pool->counter)
		{
			condition_wait(&pool->ready);
		}
	}
	condition_unlock(&pool->ready);
	condition_destroy(&pool->ready);
}

