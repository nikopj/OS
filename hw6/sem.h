#ifndef __SEM_H
#include <sys/types.h>

#define MAX_WAIT 64
struct sem
{
	volatile int count;
	volatile pid_t waitstack[MAX_WAIT];
	volatile int waitlen;
	volatile char waitlock;
	volatile char countlock;
};
void sem_init(struct sem *s, int count);
int sem_try(struct sem *s);
void sem_wait(struct sem *s);
void sem_inc(struct sem *s);
#define __SEM_H
#endif
