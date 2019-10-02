#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
void* func(void* arg)
{
	int i = 0;
	for (; i < 5; i++)
	{
		printf("func run%d\n", i);
		sleep(1);
	}
	int* p = (int*)malloc(sizeof(int));
	*p = 11;
	return p;
}
int main()
{
	pthread_t t1, t2;
	int err = pthread_create(&t1, NULL, func, NULL);
	if (err != 0)
	{
		printf("thread_create Failed:%s\n", strerror(errno));
	}
	else 
	{
		printf("thread_create success\n");
	}
	void* p = NULL;
	pthread_join(t1, &p);
	printf("线程退出：code=%d\n", *(int*)p);
	return EXIT_SUCCESS;
}
