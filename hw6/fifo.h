#ifndef __FIFO_H
#include "sem.h"

#define MYFIFO_BUFSIZ 0x1000
struct fifo
{
	struct sem sfifo_rd;
	struct sem sfifo_wr;
	struct sem smutex_rd;
	struct sem smutex_wr;
	unsigned long buf[MYFIFO_BUFSIZ];
	int writr;
	int rditr;
};
void fifo_init(struct fifo *f);
void fifo_wr(struct fifo *f, unsigned long d);
unsigned long fifo_rd(struct fifo *f);
#define __FIFO_H
#endif
