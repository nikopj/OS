#include "tas.h"
#include "spinlib.h"
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef N_PROC
#define N_PROC 64
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

struct sem
{
	volatile int count;
	volatile pid_t waitlist[N_PROC];
	volatile char waitlock;
	volatile char countlock;
};

void handler(int sn){}

static void block(struct sem *s, int n_vp)
{
	sigset_t blk_mask, empty_mask;
	sigaddset(&blk_mask, SIGUSR1);
	sigemptyset(&empty_mask);
		
	spin_lock(&s->countlock);
		if(s->count>0)
		{
			spin_unlock(&s->countlock);
			return;
		}
	spin_lock(&s->waitlock);
		sigprocmask(SIG_BLOCK, &blk_mask, NULL);
		s->waitlist[n_vp]=getpid();
	spin_unlock(&s->waitlock);
	spin_unlock(&s->countlock);

	sigsuspend(&empty_mask);
}

static void wake(struct sem *s)
{
	int i;
	spin_lock(&s->waitlock);
		for(i=0;i<N_PROC;i++)
		{
			if(s->waitlist[i]>0)
			{
				kill(s->waitlist[i],SIGUSR1);
				s->waitlist[i]=0;
			}
		}
	spin_unlock(&s->waitlock);
}

void sem_init(struct sem *s, int count)
{
	s->count = count;
	s->countlock = 0;
	s->waitlock = 0;
	signal(SIGUSR1,handler);
}

int sem_try(struct sem *s)
{
	spin_lock(&s->countlock);
		if(s->count<1)
		{
			spin_unlock(&s->countlock);
			return 0;
		}
		s->count--;
	spin_unlock(&s->countlock);
	return 1;
}

void sem_wait(struct sem *s, int n_vp)
{
	while(!sem_try(s))
		block(s, n_vp);
}

void sem_inc(struct sem *s)
{
	spin_lock(&s->countlock);
		s->count++;
		if(s->count==1)
			wake(s);
	spin_unlock(&s->countlock);
}
