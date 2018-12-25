#include "fifo.h"

void fifo_init(struct fifo *f)
{
	sem_init(&f->sfifo_wr,MYFIFO_BUFSIZ-1);
	sem_init(&f->sfifo_rd,0);
	sem_init(&f->smutex_wr,1);
	sem_init(&f->smutex_rd,1);
	f->writr=0; f->rditr=0;
}

void fifo_wr(struct fifo *f, unsigned long d)
{
	sem_wait(&f->smutex_wr);
		sem_wait(&f->sfifo_wr);
		f->buf[f->writr++] = d;
		f->writr %= MYFIFO_BUFSIZ;	
		sem_inc(&f->sfifo_rd);
	sem_inc(&f->smutex_wr);
}

unsigned long fifo_rd(struct fifo *f)
{
	unsigned long ret;
	sem_wait(&f->smutex_rd);
		sem_wait(&f->sfifo_rd);
		ret = f->buf[f->rditr++];
		f->rditr %= MYFIFO_BUFSIZ;
		sem_inc(&f->sfifo_wr);
	sem_inc(&f->smutex_rd);
	return ret;
}
