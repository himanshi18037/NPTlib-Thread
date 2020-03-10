//apt-get install gcc-multilib

#include "thread.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


// thread metadata
struct thread {
	void *esp;
	struct thread *next;
	struct thread *prev;
};

struct thread *ready_list = NULL;     // ready list
struct thread *cur_thread = NULL;     // current thread
struct thread *sleeping_list = NULL;

// defined in context.s
void context_switch(struct thread *prev, struct thread *next);

// insert the input thread to the end of the ready list.
static void push_back(struct thread *t)
{
	t->next = NULL;
	t->prev = NULL;
	struct thread * temp = ready_list;
	// printf("temp %p\n", temp);
	if(temp!=NULL){
		// printf("start %p\n", temp);
		// int r;
		// int p = scanf("%d", &r);
		// r = 1;
		while(temp->next!=NULL){
			temp=temp->next;
			// printf("temp hu m %p\n",temp);
		}
		temp->next=t;
		t->prev=temp;
	}
	else{
		// printf("first time\n");
		ready_list=t;
		ready_list->next = NULL;
	}
	
}

static void push_back_lock(struct thread *wait_list)
{

	struct thread * temp = wait_list;
	if(temp!=NULL){
		while(temp->next!=NULL){
			temp=temp->next;
		}
		temp->next=cur_thread;
		cur_thread->prev=temp;
	}
	else{
		printf("in else\n");
		wait_list=cur_thread;
		cur_thread->next = NULL;
	}
	
}

// remove the first thread from the ready list and return to caller.
static struct thread *pop_front()

{
	struct thread * temp=ready_list;
	ready_list=ready_list->next;
	return temp;
}

// the next thread to schedule is the first thread in the ready list.
// obtain the next thread from the ready list and call context_switch.
static void schedule()
{
	// printf("ohhh laaaa scedule hu m\n" );
struct thread *prev = cur_thread;
struct thread *next = pop_front();
cur_thread = next;
context_switch(prev, next);

}

// push the cur_thread to the end of the ready list and call schedule
// if cur_thread is null, allocate struct thread for cur_thread
static void schedule1()
{
	// printf("ohhh laaaa\n" );
	if(cur_thread==NULL){
		int size=sizeof(cur_thread);
		cur_thread=(struct thread *)malloc(size);
	}
	push_back(cur_thread);
	schedule();
}

// allocate stack and struct thread for new thread
// save the callee-saved registers and parameters on the stack
// set the return address to the target thread
// save the stack pointer in struct thread
// push the current thread to the end of the ready list
void create_thread(func_t func, void *param)
{
// printf("dfghmhgfghhjhjj\n" );
unsigned *stack = malloc(4096);
stack += 1024;
struct thread *t = malloc(sizeof(struct thread));
stack-=1;
*(void * *)stack =param;
stack-=1;
*stack=0;
stack-=1;
*(func_t* )stack=func;
stack-=1;
*stack=0;
stack-=1;
*stack=0;
stack-=1;
*stack=0;
stack-=1;
*stack=0;

t->esp=stack;
push_back(t);

}

// call schedule1
void thread_yield()
{
	schedule1();
}

// call schedule
void thread_exit()
{
	schedule();
}

// call schedule1 until ready_list is null
void wait_for_all()
{
	while(ready_list!=NULL ){
		schedule1();
	}
	if(ready_list==NULL ){

	}
}


void sleep(struct lock *lock)
{
	if(sleeping_list==NULL){
		sleeping_list = cur_thread;
		cur_thread->next = NULL;
	}
	else{
		struct thread * temp = sleeping_list;
		if(temp!=NULL){
			while(temp->next!=NULL){
				temp=temp->next;
			}
			temp->next=cur_thread;
			cur_thread->prev=temp;
		}
	}
	struct thread * temp = lock->wait_list;
	if(temp!=NULL){
		while(temp->next!=NULL){
			temp=temp->next;
		}
		temp->next=cur_thread;
		cur_thread->prev=temp;
	}
	else{
		lock->wait_list = cur_thread;
		cur_thread->next = NULL;
	}
	// printf("wait_list of lock %p\n",lock->wait_list);
	schedule();
}

void wakeup(struct lock *lock)
{	

	if(lock->wait_list!=NULL){
		struct thread * temp=lock->wait_list;
		temp->next=NULL;
		temp->prev=NULL;
		lock->wait_list=((struct thread*)lock->wait_list)->next;
		if(lock->wait_list!=NULL){
			((struct thread*)lock->wait_list)->prev=NULL;
		}
		push_back(temp);
	}
}
