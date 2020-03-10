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
	struct lock * lock;
	void * sp;
};

struct thread *ready_list = NULL;     // ready list
struct thread *cur_thread = NULL;     // current thread
struct thread *sleeping_list = NULL;
struct thread * exit_check=NULL;

// defined in context.s
void context_switch(struct thread *prev, struct thread *next);

// insert the input thread to the end of the ready list.
static void push_back(struct thread *t)
{
	t->next = NULL;
	t->prev = NULL;
	struct thread * temp = ready_list;
	if(temp!=NULL){
		// printf("second timeeeeee time\n");
		while(temp->next!=NULL){
			temp=temp->next;
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


// remove the first thread from the ready list and return to caller.
static struct thread *pop_front()

{   
	// printf("readyyyyyyyyyyyyyy before   %p\n",ready_list);
	struct thread * temp=ready_list;
	ready_list=ready_list->next;
	// printf("readyyyyyyyyyyyyyy after   %p\n",ready_list);

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
	// printf("dfsgdhfg\n");
	// printf("ohhh laaaa\n" );
	if(cur_thread==NULL){
		// printf("ohhh laaaa\n" );
		int size=sizeof(cur_thread);
		cur_thread=(struct thread *)malloc(size);
	}
	// printf("push called in scedule1 \n");
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
struct thread *t = malloc(sizeof(struct thread));
unsigned *stack = malloc(4096);
t->sp=stack;
stack += 1024;
// struct thread *t = malloc(sizeof(struct thread));
printf("mallloc ka ptr.   %p\n",t);
printf("stack vala ptr %p\n", stack);

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
	if(exit_check!=NULL){
		printf("free k sp %p\n", exit_check->sp);
		printf("free k thread %p\n", exit_check);
		fflush(NULL);
		free(exit_check->sp);
		free(exit_check);

	}
	exit_check=cur_thread;

	schedule();
}

// call schedule1 until ready_list is null
void wait_for_all()
{
	while(ready_list!=NULL ){
		schedule1();
	}
	while(ready_list==NULL || sleeping_list!=NULL){
		struct thread* temp=sleeping_list;
		// printf("temp hu m %p\n",temp);
		if(temp!=NULL){
			sleeping_list=sleeping_list->next;
			if(sleeping_list!=NULL){
				sleeping_list->prev=NULL;
			}
		}
		struct lock *lock;
		if(temp!=NULL){
			lock=temp->lock;
			temp->lock->val=1;
		}

		// printf("lock ki waitlist hu mmmmmmmmm    %p\n",lock->wait_list);
		// printf("lock hu m %p\n",lock);
		struct thread * pre;
		struct thread * curr;
		printf("lock hu mmmmmm %p\n",lock);
		if(lock==NULL){
			printf("yipeeeeeeeee\n");
		}
		if(lock!=NULL){
			if(lock->wait_list!=NULL){
			 pre=((struct thread *)lock->wait_list)->prev;
			 curr=lock->wait_list;
		}
		}
		
		// printf("preeeee %p\n",pre);
		
		while(curr!=NULL){
			if(temp==curr){
				if(pre==NULL){
					// printf("yhssssssssss\n");
					lock->wait_list=((struct thread *)lock->wait_list)->next;
					printf("lock sfter head %p\n",lock->wait_list);
					if(lock->wait_list!=NULL){
						((struct thread *)lock->wait_list)->prev=NULL;
					}
					curr->next=NULL;
					curr=lock->wait_list;
					pre=NULL;
					continue;
				}
				else{
					// printf("yhsssssssssssssssssssssssssssssssss\n");
					pre->next=curr->next;
					if(curr->next!=NULL){
						curr->next->prev=pre;
					}
					curr->prev=NULL;
					curr=curr->next;
					continue;
				}
			}
			pre=curr;
			curr=curr->next;
		}
		
		// printf("lock ki waitlist hu m     %p\n",lock->wait_list);
		if(temp!=NULL){
			// printf("push calll \n");
			push_back(temp);

		}
		schedule1();
		// printf("readyyyyyyyyyyyyyy    %p\n",ready_list);
		
		
	}
}


void sleep(struct lock *lock)
{
	cur_thread->lock=lock;
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
	printf("wait_list of lock %p\n",lock->wait_list);
	if(lock->wait_list!=NULL){
		struct thread * temp=lock->wait_list;
		temp->next=NULL;
		temp->prev=NULL;
		struct thread * pre=NULL;
		struct thread * curr=NULL;
		// printf("sleeeeee %p\n",sleeping_list );
		if(sleeping_list!=NULL){
			// printf("hereeeeee\n");
			 pre=sleeping_list->prev;
			 curr=sleeping_list;
		}
		// printf("sleeeeee %p\n",sleeping_list );
		// printf("currrrrrrrr %p\n",curr );
		while(curr!=NULL){
			// printf("currrrrrrrrrrrrrr %p\n",curr );
			if(temp==curr){
				// printf("athe hu m\n");
				if(pre==NULL){
					sleeping_list=sleeping_list->next;
					if(sleeping_list!=NULL){
						sleeping_list->prev=NULL;
					}
					curr->next=NULL;
					curr=sleeping_list;
					pre=NULL;
					continue;
				}
				else{
					pre->next=curr->next;
					if(curr->next!=NULL){
						curr->next->prev=pre;
					}
					curr->prev=NULL;
					curr=curr->next;
					continue;
				}
			}
			pre=curr;
			curr=curr->next;
		}
		lock->wait_list=((struct thread*)lock->wait_list)->next;
		if(lock->wait_list!=NULL){
			((struct thread*)lock->wait_list)->prev=NULL;
			
			
		}
		push_back(temp);
		
		
		// printf("push called in wakeup\n");
		
	}
}
