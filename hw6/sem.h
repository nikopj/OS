#ifndef __SEM_H

#ifndef N_PROC
#define N_PROC 64
#endif
struct sem
{
	volatile int count;
	volatile pid_t waitlist[N_PROC];
	volatile char waitlock;
	volatile char countlock;
};
void handler(int sn);
void sem_init(struct sem *s, int count, int wait_len);
int sem_try(struct sem *s);
void sem_wait(struct sem *s);
void sem_inc(struct sem *s);
#define __SEM_H
#endif
