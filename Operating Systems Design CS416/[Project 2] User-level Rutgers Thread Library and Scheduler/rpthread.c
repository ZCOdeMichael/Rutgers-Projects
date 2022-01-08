// File:	rpthread.c

// List all group member's name: Michael Zhang, Andrew Cater 
// username of iLab: mbz27, ajc398
// iLab Server: iLab4.cs.rutgers.edu

#include "rpthread.h"
#define STACK_SIZE SIGSTKSZ
#define NUM_LEVELS 4

static uint threadID = 0; // Current thread ID
static int mutexID = 0; // Current mutex ID

static int yieldThread = 0; // If the current thread is yielding
static ucontext_t curr_sch; // The current context of the scheduler
static tcb* current = NULL; // Current thread context that should be running
static mlfq* scheduler_struct = NULL; // Structure that scheduler is using to schedule threads
static mutex_unit* mutexes = NULL; // storing all initialized mutexes

// Signal handler for timer
static struct sigaction sa;
static struct itimerval timer;
static long time_count;
static long mutex_time;

// Function to print out all queues
static void debug(){
	rr_t* head = scheduler_struct->head;
	while(head != NULL){
		tcb* ptr = head->head;
		while(ptr != NULL){
			printf("%d, %d - ", ptr->id, ptr->status);
			ptr = ptr->next;
		}
		printf("|\n");
		head = head->next;
	}
	
}

// Function that disables the timer
void disableTimer(){
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 0;
	setitimer(ITIMER_PROF, &timer, NULL);
}

// Function that adds threads to the first level queue
void addQueue(ucontext_t new, rpthread_t* thread, void* exitStack, void* conStack){
	threadID++;
	tcb* newThread = (tcb*)malloc(sizeof(tcb));
	newThread->id = threadID;
	newThread->status = READY;
	newThread->level = 1;
	newThread->context = new;
	newThread->exit_Stack = exitStack;
	newThread->con_Stack = conStack;
	newThread->retValue = NULL;
	newThread->next = NULL;

	if(scheduler_struct->head->head == NULL){		
		scheduler_struct->head->head = newThread;
		scheduler_struct->head->tail = newThread;
		scheduler_struct->head->num_thread++;
		scheduler_struct->total_thread++;
	}else{
		scheduler_struct->head->tail->next = newThread;
		scheduler_struct->head->tail = newThread;
		scheduler_struct->head->num_thread++;
		scheduler_struct->total_thread++;
	}

	*thread = newThread->id;
}

// Signal handler that yields the current thread that is running
void sigHandler(int signum){
	rpthread_yield();
}

// Function that initializes and allocate memory to all necessary data structures for either RR or MLFQ
void init(){
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &sigHandler;
	sigaction(SIGPROF, &sa, NULL);

	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = TIMESLICE;
	timer.it_value.tv_sec = 0;

	#ifndef MLFQ
	// Choose RR
	scheduler_struct = (mlfq*)malloc(sizeof(mlfq));
	scheduler_struct->head = (rr_t*)malloc(sizeof(rr_t));
	scheduler_struct->total_thread = 0;
	rr_t* temp = scheduler_struct->head;
	temp->num_thread = 0;
	temp->level = 1;
	temp->head = NULL;
	temp->tail = NULL;
	scheduler_struct->head->next = NULL;

	#else 
	// Choose MLFQ
	scheduler_struct = (mlfq*)malloc(sizeof(mlfq));
	scheduler_struct->total_thread = 0;
	
	rr_t* temp1 = (rr_t*)malloc(sizeof(rr_t));
	temp1->num_thread = 0;
	temp1->level = NUM_LEVELS;
	temp1->head = NULL;
	temp1->tail = NULL;
	temp1->next = NULL;
	scheduler_struct->head = temp1;

	int i;
	for(i = NUM_LEVELS-1; i > 0; i--){
		rr_t* temp2 = (rr_t*)malloc(sizeof(rr_t));
		temp2->num_thread = 0;
		temp2->level = i;
		temp2->head = NULL;
		temp2->tail = NULL;
		temp2->next = scheduler_struct->head;
		scheduler_struct->head = temp2;
	
	}
	#endif
}

// Function that creates pthreads and calls init() if ran for the first time
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {

	// Initializes and allocates memory to all data structures for the first time
	if(scheduler_struct == NULL){
		init();
	}

	// Creates the new thread context
	ucontext_t newThreadCon, exitCon;
	getcontext(&exitCon);
	getcontext(&newThreadCon);
	void* stack1 = malloc(STACK_SIZE);
	void* stack2 = malloc(STACK_SIZE);
	
	exitCon.uc_link = NULL;
    exitCon.uc_stack.ss_sp = stack1;
    exitCon.uc_stack.ss_size = STACK_SIZE;
    exitCon.uc_stack.ss_flags = 0;

	makecontext(&exitCon, (void *)&rpthread_exit, 0);

	newThreadCon.uc_link = &exitCon; 
    newThreadCon.uc_stack.ss_sp = stack1;
    newThreadCon.uc_stack.ss_size = STACK_SIZE;
    newThreadCon.uc_stack.ss_flags = 0;

	makecontext(&newThreadCon, (void*)function, 1, arg);

	// enqueues the newly created thread
	addQueue(newThreadCon, thread, stack1, stack2);

    return 0;
};

// Function that stops and saves the current thread context and calls the scheduler
int rpthread_yield() {

	// Keeps track of the time on the timer in cases when the thread yields CPU early	
	getitimer(ITIMER_PROF, &timer);
	time_count = timer.it_value.tv_usec;
	
	disableTimer();
	yieldThread = 1; // Sets yield flag for scheduler
	schedule();
	
	// swapcontexts with context of schedule
	swapcontext(&current->context, &curr_sch);
	
	return 0;
};

// Function that terminates the current thread context and unlock mutexes locked by the thread
void rpthread_exit(void *value_ptr) {
	
	disableTimer();

	// Unlocks and mutexes locked by the current thread
	mutex_unit* ptr = mutexes;
	while(ptr != NULL){ // Unlock and mutexes locked by the current thread
		if(ptr->lock != NULL){
			if(ptr->lock->id == current->id){
				block_LL* block_Head = ptr->head;
				if(block_Head != NULL){
					tcb* thread_Lock = block_Head->thread;
					
					block_LL* temp = block_Head->next;
					free(block_Head);
					ptr->head = temp;

					ptr->lock = thread_Lock;
					thread_Lock->status = READY;
					
				}else{
					ptr->src->status = UNLOCKED;
					
				}
			}
		}
		ptr = ptr->next;
	}
	
	// Saves return value
	current->retValue = value_ptr;

	//Sets the status of the thread to TERMINATED
	current->status = TERMINATED;

	// swapcontext to scheduler
	swapcontext(&current->context, &curr_sch);
};

// Function to check if a specified thread was terminated or not
int finishedRunning(rpthread_t thread){
	rr_t* rr_ptr = scheduler_struct->head;
	while(rr_ptr != NULL){
		tcb* tcb_ptr = rr_ptr->head;
		while(tcb_ptr != NULL){
			if(tcb_ptr->id == thread){
				if(tcb_ptr->status == TERMINATED){
					return 1;
				}else{
					return 0;
				}
			}
			tcb_ptr = tcb_ptr->next;
		}
		rr_ptr = rr_ptr->next;
	}
	return 0;
}

// Function that deallocates the specified thread after joining on it
void deallocate(rpthread_t thread){
	int totalThreads = scheduler_struct->total_thread;

	rr_t* rr_ptr = scheduler_struct->head;
	while(rr_ptr != NULL){ // Goes through all mflq levels
		tcb* tcb_ptr = rr_ptr->head;
		tcb* prev = NULL;
		while(tcb_ptr != NULL){ // Goes through all threads in current level
			if(tcb_ptr->id == thread){ // Gets the thread with same id
				if(prev == NULL){ // If the thread is the head of the queue
					rr_ptr->head = tcb_ptr->next;
					if(tcb_ptr->id == rr_ptr->tail->id){ // If the thread is the tail and the head of the queue thus the only thread left in the level
						rr_ptr->tail = prev;
					}
					rr_ptr->num_thread--;
					free(tcb_ptr->con_Stack);
					free(tcb_ptr->exit_Stack);
					free(tcb_ptr);
					break;

				}else{ // If thread is not the head of the queue
					prev->next = tcb_ptr->next; 
					if(tcb_ptr->id == rr_ptr->tail->id){ // If the thread is the tail of the queue
						rr_ptr->tail = prev;
					}
					rr_ptr->num_thread--;
					free(tcb_ptr->con_Stack);
					free(tcb_ptr->exit_Stack);
					free(tcb_ptr);
					break;		
					
				}
			}
			prev = tcb_ptr;
			tcb_ptr = tcb_ptr->next;
		}
		rr_ptr = rr_ptr->next;
	}

	// Checks if this was the last thread to run
	if((totalThreads-1) == 0){ // Only one thread left thus finished all threads
		rr_t* r_ptr = scheduler_struct->head;
		while(r_ptr != NULL){ // Deallocates all levels
			rr_t* temp = r_ptr->next;
			free(r_ptr);
			r_ptr = temp;
		}
		free(scheduler_struct); // Deallocates scheduler structure
		scheduler_struct = NULL;

	}else{ // More than one thread left in the scheduler
		scheduler_struct->total_thread--;

	}
}

// Function that waits for the specified thread to TERMINATE
int rpthread_join(rpthread_t thread, void **value_ptr) {
	
	// Continuously checks if thread id is termianted or not
	while(!finishedRunning(thread)){ 
		yieldThread = 0;
		schedule(); // Runs next thread

	}

	// Sets the return value ptr if needed
	if(value_ptr){
		*value_ptr = current->retValue;
	}

	deallocate(thread); // Deallocates any dynamic memory the thread used
	
	return 0;
};


// Function that initializes and mallocs the mutex structure
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex
	mutexID++;
	mutex->id = mutexID;
	mutex->status = UNLOCKED;
	if(mutexes == NULL){
		mutex_unit* temp = (mutex_unit*)malloc(sizeof(mutex_unit));
		temp->src = mutex;
		temp->lock = NULL;
		temp->head = NULL;
		temp->next = NULL;
		mutexes = temp;
	}else{
		mutex_unit* temp = (mutex_unit*)malloc(sizeof(mutex_unit));
		temp->src = mutex;
		temp->lock = NULL;
		temp->head = NULL;
		temp->next = mutexes;
		mutexes = temp;
	}
	
	return 0;
};

// Function that will set the specified mutex to a lock state
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
	// context switch to the scheduler thread
	getitimer(ITIMER_PROF, &timer);
	mutex_time = timer.it_value.tv_usec;
	disableTimer();
	
	// Locks mutex
	mutex_unit* ptr = mutexes;
	while(ptr != NULL){
		if(ptr->src->id == mutex->id){
			if(ptr->src->status == LOCKED){ // If mutex is locked already
				current->status = BLOCKED; // Block current thread
				
				// Add it to blocked list
				if(ptr->head == NULL){
					block_LL* temp = (block_LL*)malloc(sizeof(block_LL));
					temp->thread = current;
					temp->next = NULL;
					ptr->head = temp;
				}else{
					block_LL* temp = (block_LL*)malloc(sizeof(block_LL));
					temp->thread = current;
					temp->next = ptr->head;
					ptr->head = temp;
				}
				timer.it_value.tv_usec = mutex_time;
				timer.it_value.tv_sec = 0;
				setitimer(ITIMER_PROF, &timer, NULL);
				rpthread_yield(); // Yield thread since the thread is blocked
				return 0;
			}else{ // If mutex is unlocked
				ptr->src->status = LOCKED;
				ptr->lock = current;


			}
			break;
		}

		ptr = ptr->next;
	}

	timer.it_value.tv_usec = mutex_time;
	timer.it_value.tv_sec = 0;
	setitimer(ITIMER_PROF, &timer, NULL);

	return 0;
};

// Function that will release a lock on the specified mutex
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.
	getitimer(ITIMER_PROF, &timer);
	mutex_time = timer.it_value.tv_usec;
	disableTimer();
	
	// Realease a lock on specified mutex
	mutex_unit* ptr = mutexes;
	while(ptr != NULL){
		if(ptr->src->id == mutex->id){
			block_LL* block_Head = ptr->head;
			if(block_Head != NULL){ // If there are other threads that requested mutex lock
				tcb* thread_Lock = block_Head->thread;
				
				block_LL* temp = block_Head->next;
				free(block_Head);
				ptr->head = temp;

				ptr->lock = thread_Lock;
				thread_Lock->status = READY;
				break;
			}else{ // If there are no other threads that requested mutex lock
				ptr->src->status = UNLOCKED;
				break;
			}

		}
		ptr = ptr->next;
	}

	timer.it_value.tv_usec = mutex_time;
	timer.it_value.tv_sec = 0;
	setitimer(ITIMER_PROF, &timer, NULL);

	return 0;
};

// Function that will destroy and deallocate any data structure holding the specified mutex
int rpthread_mutex_destroy(rpthread_mutex_t *mutex) {
	disableTimer();
	mutex_unit* ptr = mutexes;
	mutex_unit* prev = NULL;
	
	// deallocates data structure
	while(ptr != NULL){
		if(ptr->src->id == mutex->id){		
			block_LL* b_ptr = ptr->head;
			while(b_ptr != NULL){ // Unblocks all threads in the block list when mutex is destroyed
				b_ptr->thread->status = READY;
				block_LL* temp = b_ptr->next;
				free(b_ptr);
				b_ptr = temp;
			}
			if(prev == NULL){
				mutex_unit* temp = ptr->next;
				free(ptr);
				mutexes = temp;
			}else{
				prev->next = ptr->next;
				free(ptr);
			}
			break;
		}
		prev = ptr;
		ptr = ptr->next;
	}
	return 0;
};

// Scheduler that runs thread contexts
static void schedule() {

// schedule policy
#ifndef MLFQ
	// Choose RR
	sched_rr();
#else 
	// Choose MLFQ
	sched_mlfq();
#endif

}



// Schedules thread context in round robin order
static void sched_rr() {
	
	if(yieldThread){ // When the yield function was called

		// Pushes current thread to the back of the queue
		rr_t* ptr = scheduler_struct->head;
		if(ptr->head->id != ptr->tail->id){
			tcb* temp = ptr->head;
			ptr->head = ptr->head->next;
			temp->next = NULL;
			ptr->tail->next = temp;
			ptr->tail = temp;
		}
		yieldThread = 0;
	}else{ // Not yielding

		// Picks and schedules a thread that is not TERMINATED and not BLOCKED
		rr_t* ptr = scheduler_struct->head;
		while(ptr->head->status == TERMINATED || ptr->head->status == BLOCKED){
			if(ptr->head->id != ptr->tail->id){
				tcb* temp = ptr->head;
				ptr->head = ptr->head->next;
				temp->next = NULL;
				ptr->tail->next = temp;
				ptr->tail = temp;
			}
		}
		current = ptr->head;
		current->status = SCHEDULED;

		// Sets timer
		timer.it_value.tv_usec = TIMESLICE;
		timer.it_value.tv_sec = 0;
		setitimer(ITIMER_PROF, &timer, NULL);
		
		// swapcontext to thread context
		swapcontext(&curr_sch, &(current->context));
	}
}


// Function that gets the top thread that's not TERMINATED and not BLOCKED
static tcb* getTop(){
	rr_t* head = scheduler_struct->head;
	while(head != NULL){
		tcb* ptr = head->head;
		while(ptr != NULL){
			if(ptr->status != TERMINATED && ptr->status != BLOCKED){
				return ptr;
				
			}
			ptr = ptr->next;
		}
		head = head->next;
	}
	return NULL;
}

// Function that demotes the level of the current thread when taking entire timeslice
static void demoteLevel(){
	rr_t* head = scheduler_struct->head;
	rr_t* prev = NULL;
	while(head != NULL){
		if(current->level == head->level){
			if(head->head == NULL){ // No threads in queue
				head->head = current;
				head->tail = current;
				head->num_thread++;
				return;
			}else{ // Threads in queue
				head->tail->next = current;
				head->tail = current;
				head->num_thread++;
				return;
				
			}
		}
		prev = head;
		head = head->next;
	}

}

// Function that pushes current thread to the back of the queue in its level
static void pushBack(){
	rr_t* head = scheduler_struct->head;
	while(head != NULL){
		tcb* ptr = head->head;
		tcb* prev = NULL;
		while(ptr != NULL){
			if(ptr->id == current->id){ // Finds current thread that ran
				if(prev == NULL){ // ptr is Head of queue
					if(ptr->id != head->tail->id){ // ptr is not be tail of queue but is the head of the queue
						tcb* temp = ptr;
						head->head = head->head->next;
						temp->next = NULL;
						head->tail->next = temp;
						head->tail = temp;
					}
					return;
				}else{ // Not head of queue
					if(ptr->id != head->tail->id){ // ptr is not the tail or head of the queue
						prev->next = ptr->next;
						ptr->next = NULL;
						head->tail->next = ptr;
						head->tail = ptr;
					}
					return;
				}	
			}
			prev = ptr;
			ptr = ptr->next;
		}
		head = head->next;
	}
}

// Schedules thread contexts in MLFQ order
static void sched_mlfq() {
	if(yieldThread){ // The thread has yield
		if(time_count != 0){ // Thread took less than the time slice
			
			pushBack();
			return;
		}else{ // Thread took entire time slice
			
			rr_t* head = scheduler_struct->head;
			while(head != NULL){
				tcb* ptr = head->head;
				tcb* prev = NULL;
				while(ptr != NULL){
					if(current->level == NUM_LEVELS){ // Current thread is at the last level in the MLFQ
						pushBack();
						return;
					}else{ // Current thread is not at the last level in the MLFQ 1 to (NUM_LEVELS-1)
						if(ptr->id == current->id){
							if(prev == NULL){ // Head of queue
								if(ptr->id == head->tail->id){ // ptr is the tail and head of the queue								
									head->head = NULL;
									head->tail = NULL;
									current->next = NULL;
									current->level++;
									head->num_thread--;
									demoteLevel();
								}else{ // ptr is the head but not the tail
									head->head = head->head->next;
									current->next = NULL;
									current->level++;
									head->num_thread--;
									demoteLevel();
								}
								return;
							}else{
								if(ptr->id == head->tail->id){ // ptr is the tail but not the head of the queue 
									prev->next = NULL;
									head->tail = prev;
									current->next = NULL;
									current->level++;
									head->num_thread--;
									demoteLevel();

								}else{ // ptr is not the head or the tail
									prev->next = ptr->next;
									current->next = NULL;
									current->level++;
									head->num_thread--;
									demoteLevel();
								}
								return;
							}
						}
					}

					prev = ptr;
					ptr = ptr->next;
				}
				head = head->next;
			}

		}
	}else{
		// Gets and schedules the top thread
		tcb* curr = getTop();
		curr->status = SCHEDULED;
		current = curr;

		// Sets timer
		timer.it_value.tv_usec = TIMESLICE;
		timer.it_value.tv_sec = 0;
		setitimer(ITIMER_PROF, &timer, NULL);
	
		// swapcontext to thread context
		swapcontext(&curr_sch, &(current->context));
	
	}
}



