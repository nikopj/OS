#include "tas.h"
#include "spinlib.h"

struct sem
{
	int count;
	volatile char spinlock;
	struct sigaction sa;
}

void sem_init(struct sem *s, int count)
{
	s->count = count;
	s->spinlock = 0;
	sa.sa_handler = wakeup;
}

int sem_try(struct sem *s)
{
	return !tas(s->spinlock);
}

void sem_wait(struct sem *s)
{
	spin_lock(s->lock);
	s->count--;
	spin_unlock(s->lock);
}

void sem_inc(struct sem *s);
