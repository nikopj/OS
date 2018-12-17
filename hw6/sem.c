#include "tas.h"
#include "spinlib.h"
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#ifndef N_PROC
#define N_PROC 64
#endif

struct sem
{
	int count;
	pid_t waitlist[N_PROC];
	volatile char waitlock;
	volatile char countlock;
}

static void block(struct sem *s, int n_vp)
{
	sigset_t mask, oldmask;
	sigaddset(&mask, SIGUSR1);

	spin_lock(s->waitlock);
		sigprocmask(SIG_BLOCK, &mask, &oldmask);
		waitlist[n_vp]=getpid();
	spin_unlock(s->waitlock);

	sigsuspend(&oldmask);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

static void wake(struct sem *s)
{
	int i;
	spin_lock(s->waitlock);
		for(i=0;i<(sizeof(s->waitlist)/sizeof(pid_t));i++)
		{
			if(waitlist[i]>0)
				kill(waitlist[i],SIGUSR1);
				waitlist[i]=0;
		}
	spin_unlock(s->waitlock);
}

void sem_init(struct sem *s, int count)
{
	s->count = count;
	s->countlock = 0;
	s->waitlock  = 0;
}

int sem_try(struct sem *s)
{
	if(s->count<1)
		return 0;
	spin_lock(s->countlock);
		count--;
	spin_unlock(s->countlock);
	return 1;
}

void sem_wait(struct sem *s, int n_vp)
{
	while(!sem_try(s))
		block(s, n_vp);
}

void sem_inc(struct sem *s)
{
	int oldcount;
	spin_lock(s->countlock);
		oldcount = s->count;
		s->count++;
		if(oldcount<1)
			wake(s);
	spin_unlock(s->countlock);
}
