#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_test/so_scheduler.h"

#define RETURN_ERROR -1
#define MAX_THREADS 1000

typedef struct {
	unsigned int time_quantum;
	unsigned int io;
} scheduler;

typedef struct {
	HANDLE tid;
	unsigned int priority;
	unsigned int time_quantum;
	char *state;
} thread;

typedef struct {
	so_handler *handler;
	unsigned int priority;
	thread *thr;
} params;

scheduler *sch;
int sched_initialised = 0, no_threads = 0, running_thread_id;

thread *threads[MAX_THREADS];
params *param_list[MAX_THREADS];

DECL_PREFIX int so_init(unsigned int time_quantum, unsigned int io)
{
	if (io > SO_MAX_NUM_EVENTS || time_quantum == 0)
		return RETURN_ERROR;

	if (sched_initialised == 1)
		return RETURN_ERROR;

	sch = calloc(sizeof(scheduler), 1);

	if (sch == NULL)
		return RETURN_ERROR;

	sch->time_quantum = time_quantum;
	sch->io = io;
	sched_initialised = 1;
	return 0;
}

void *start_thread(void *args)
{
	params *p = (params *) args;

	p->thr->state = "RUNNING";
	p->handler(p->priority);
	p->thr->tid = GetCurrentThread();

	return NULL;
}

DECL_PREFIX tid_t so_fork(so_handler *func, unsigned int priority)
{
	thread *thr;
	params *p;
	DWORD tid;

	if (func == NULL || priority > SO_MAX_PRIO)
		return INVALID_TID;

	thr = calloc(sizeof(thread), 1);

	if (thr == NULL)
		return INVALID_TID;

	thr->state= "NEW";
	thr->priority = priority;
	thr->time_quantum = sch->time_quantum;

	p = calloc(sizeof(params), 1);

	if (p == NULL)
		return INVALID_TID;

	p->handler = func;
	p->priority = priority;
	p->thr = thr;

	threads[no_threads] = thr;
	running_thread_id = no_threads;
	param_list[no_threads++] = p;

	thr->tid = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)start_thread, p, 0, &tid);
	WaitForSingleObject(thr->tid, INFINITE);

	return tid;
}

DECL_PREFIX int so_wait(unsigned int io)
{
	return (io < 0 || io >= sch->io) ? -1 : 0;
}

DECL_PREFIX int so_signal(unsigned int io)
{
	return (io < 0 || io >= sch->io) ? -1 : running_thread_id;
}

DECL_PREFIX void so_exec(void)
{

}

DECL_PREFIX void so_end(void)
{
	sched_initialised = 0;
	free(sch);
}
