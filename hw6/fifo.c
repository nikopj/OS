#include "sem.h"

struct fifo
{
	struct sem *sfifo;
	struct sem *smutex;
}


void fifo_init(struct fifo *f)
{
	sem_init(sfifo,

}

void fifo_wr(struct fifo *f, unsigned long d)
{

}

unsigned long fifo_rd(struct fifo *f)
{

}
