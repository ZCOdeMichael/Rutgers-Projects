// File:	rpthread_t.h

// List all group member's name: Michael Zhang, Andrew Cater 
// username of iLab: mbz27, ajc398
// iLab Server: iLab4.cs.rutgers.edu

#ifndef RTHREAD_T_H
#define RTHREAD_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_RTHREAD macro */
#define USE_RTHREAD 1

#ifndef TIMESLICE
/* defined timeslice to 5 ms, feel free to change this while testing your code
 * it can be done directly in the Makefile*/
#define TIMESLICE 5
#endif

#define READY 0
#define SCHEDULED 1
#define BLOCKED 2
#define TERMINATED 3

#define LOCKED 5
#define UNLOCKED 6

#define MLFQ_LEVELS 4

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

typedef uint rpthread_t;

// Struct to contain information about thread
typedef struct threadControlBlock {
	int id; // Thread ID
	int status; // State of thread
	int level; // Level in MLFQ
	ucontext_t context; // Store context of thread
	void* retValue; // Stores return value in pthread_exit

	void* con_Stack; // Stores context stack
	void* exit_Stack; // Stores pthread_exit context stack

	struct threadControlBlock* next; // Link list of threadcontrolblocks

} tcb; 

// Struct to contain round robin
typedef struct round_robin{
	int num_thread; // Number of threads on this level
	int level; // Queue Level
	struct threadControlBlock* head; // Round Robin storing all threads
	struct threadControlBlock* tail; // Queue's tail

	struct round_robin* next; // List of Round Robins
}rr_t;

// Struct to contain multiple round robins
typedef struct mflqs{
	int total_thread; // Total number of threads currently scheduled to run
	struct round_robin* head; // List of Round Robin queues

}mlfq;

// Struct to contain all blocked threads b/c of mutex
typedef struct block_LLs{
	tcb* thread; // thread that is blocked
	struct block_LLs* next; // List of all blocked thread

} block_LL;

// Struct to contain all the mutexes
typedef struct rpthread_mutex_t {
	int id; // Id of mutex
	int status; // Status of mutex locked/unlocked
} rpthread_mutex_t;

// Struct to contain information of a mutex
typedef struct mutex_Units{
	struct rpthread_mutex_t* src; // Mutex info
	struct threadControlBlock* lock; // thread that locked the mutex
	struct block_LLs* head; // List of threads blocked by mutex

	struct mutex_Units* next; // Pointer to the next mutex
} mutex_unit;


/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, void
    *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int rpthread_yield();

/* terminate a thread */
void rpthread_exit(void *value_ptr);

/* wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr);

/* initial the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, const pthread_mutexattr_t
    *mutexattr);

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex);

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex);

/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex);

// Scheduler that runs thread contexts
static void schedule();

// Schedules thread context in round robin order
static void sched_rr();

// Schedules thread contexts in MLFQ order
static void sched_mlfq();

// Function to print out all queues
static void debug();

// Function that disables the timer
void disableTimer();

// Function that adds threads to the first level queue
void addQueue(ucontext_t, rpthread_t*, void*, void*);

// Signal handler that yields the current thread that is running
void sigHandler(int);

// Function that initializes and allocate memory to all necessary data structures for either RR or MLFQ
void init();

// Function to check if a specified thread was terminated or not
int finishedRunning(rpthread_t);

// Function that deallocates the specified thread after joining on it
void deallocate(rpthread_t);

// Function that gets the top thread that's not TERMINATED and not BLOCKED
static tcb* getTop();

// Function that demotes the level of the current thread when taking entire timeslice
static void demoteLevel();

// Function that pushes current thread to the back of the queue in its level
static void pushBack();

#ifdef USE_RTHREAD
#define pthread_t rpthread_t
#define pthread_mutex_t rpthread_mutex_t
#define pthread_create rpthread_create
#define pthread_exit rpthread_exit
#define pthread_join rpthread_join
#define pthread_mutex_init rpthread_mutex_init
#define pthread_mutex_lock rpthread_mutex_lock
#define pthread_mutex_unlock rpthread_mutex_unlock
#define pthread_mutex_destroy rpthread_mutex_destroy
#endif

#endif