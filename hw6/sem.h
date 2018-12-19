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
void debug_print(struct sem *s);
void sem_init(struct sem *s, int count);
int sem_try(struct sem *s);
void sem_wait(struct sem *s, int n_vp);
void sem_inc(struct sem *s);
#define __SEM_H
#endif
