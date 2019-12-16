#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>

struct Data
{
	int x;
	int y;
};

void* t_function(void* data)
{
	struct Data data1;
	data1 = *((struct Data*)data);
	return (void*)(data1.x + data1.y);
}

int main(void)
{
	pthread_t p_thread;
	int err;
	int status;
	struct Data data;
	data.x = 1;
	data.y = 3;

	if((err = pthread_create(&p_thread, NULL, t_function, (void*)&data)) < 0)
	{
		perror("pthread create err: ");
		exit(1);
	}

	pthread_join(p_thread, (void**)&status);
	printf("return: %d\n", status);

	return 0;
}
