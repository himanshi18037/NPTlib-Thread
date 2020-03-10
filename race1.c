#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "thread.h"

#define MAX_THREADS 1024

int counter = 0;

static int acquire(struct lock *lock)
{
	while (lock->val == 0) {
		sleep(lock);
	}
	lock->val = 0;
}

static int release(struct lock *lock)
{   
	// printf("m hu release\n");
	wakeup(lock);
	lock->val = 1;
}

void foo(void *ptr)
{
	struct lock *l = (struct lock*)ptr;
	int val;
	acquire(l);
	val = counter;
	val++;
	thread_yield();
	counter = val;
	release(l);
	thread_exit();
}

void bar(void *ptr)
{
	struct lock *l = (struct lock*)ptr;
	int val;
	acquire(l);
	// printf("bar acquires\n");
	val = counter;
	val++;
	counter = val;
	// printf("bar relases\n" );
	release(l);

	thread_exit();
}

int main(int argc, char *argv[])
{
	struct lock l;
	l.val = 1;
	l.wait_list = NULL;

	create_thread(foo, &l);
	create_thread(bar, &l);
	wait_for_all();

	assert(counter == 2);
	printf("main thread exiting.\n");

	return 0;
}
