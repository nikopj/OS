#include "tas.h"
#include "spinlib.h"
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_WAIT 64

struct node
{
	pid_t pid;
	struct node *nptr;
}

struct sem
{
	volatile int count;
	volatile pid_t waitlist[MAX_WAIT];
	int wait_len;
	volatile char waitlock;
	volatile char countlock;
};

void handler(int sn){}

static void block(struct sem *s)
{
	int i=0;
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
		while(waitlist[i]!=0)
		{
			if(i==MAX_WAIT-1)
			{
				fprintf(stderr,"error, waitlist too long!\n");
				exit(-1);
			}
			i++;
		}
		s->waitlist[i]=getpid();
	spin_unlock(&s->waitlock);
	spin_unlock(&s->countlock);

	sigsuspend(&empty_mask);
}

static void wake(struct sem *s)
{
	int i=0;
	spin_lock(&s->waitlock);
	/*
		for(i=0;i<N_PROC;i++)
		{
			if(s->waitlist[i]>0)
			{
				kill(s->waitlist[i],SIGUSR1);
				s->waitlist[i]=0;
			}
		}
	*/
		while(s->waitlist[i]!=0)
		{
			if(i==MAX_WAIT-1)
			{
				fprintf(stderr,"error, waitlist too long!\n");
				exit(-1);
			}
			kill(s->waitlist[i],SIGUSR1);
			s->waitlist[i]=0;
			i++;
		}
	spin_unlock(&s->waitlock);
}

void sem_init(struct sem *s, int count, int wait_len)
{
	s->count = count;
	s->countlock = 0;
	s->waitlock = 0;
	s->wait_len = wait_len;
	signal(SIGUSR1,handler);
	if(wait_len>MAX_WAIT)
	{
		fprintf(stderr,"wait length too large (%d>%d)\n",wait_len,MAX_WAIT);
		exit(-1);
	}
	// initialize waitlist stack to zeros
	int i=0;
	while(i<wait_len)
		waitlist[i++]=0;
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
		block(s);
}

void sem_inc(struct sem *s)
{
	spin_lock(&s->countlock);
		s->count++;
		if(s->count==1)
			wake(s);
	spin_unlock(&s->countlock);
}
