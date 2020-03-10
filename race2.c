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
	wakeup(lock);
	lock->val = 1;
}

void foo(void *ptr)
{
	struct lock *l = (struct lock*)ptr;
	acquire(l);
	// printf("foo acquires\n" );
	int val = counter;
	val++;
	counter = val;
	thread_yield();
	// printf("foo khtmmmmm\n");
	thread_exit();
	// printf("foo khtmmmmm\n");
}

void bar(void *ptr)
{
	// printf("bar chla\n" );
	struct lock *l = (struct lock*)ptr;
	acquire(l);
	// printf("bar acquires\n" );
	int val = counter;
	val++;
	counter = val;
	release(l);
	// printf("bar release\n" );

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
