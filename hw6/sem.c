#include "tas.h"
#include "spinlib.h"
#include "sem.h"
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static void handler(int sn){}

static pid_t pid_pop(struct sem *s)
{
	if(s->waitlen<1)
		return -1;
	s->waitlen--;
	return s->waitstack[s->waitlen];
}

static pid_t pid_push(struct sem *s, pid_t pid)
{
	if(s->waitlen>=MAX_WAIT)
		return -1;
	s->waitstack[s->waitlen++] = pid;
	return 0;
}

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
		if( pid_push(s, getpid())<0 )
		{
			fprintf(stderr,"pid_push(s,%d) error, stack too large\n",getpid());
			exit(-1);
		}
	spin_unlock(&s->waitlock);
	spin_unlock(&s->countlock);

	sigsuspend(&empty_mask);
}

static void wake(struct sem *s)
{
	pid_t pid;
	spin_lock(&s->waitlock);
		while( (pid=pid_pop(s))>0 )
			kill(pid,SIGUSR1);
	spin_unlock(&s->waitlock);
}

void sem_init(struct sem *s, int count)
{
	s->count = count;
	s->countlock = 0;
	s->waitlock = 0;
	s->waitlen = 0;
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

void sem_wait(struct sem *s)
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
