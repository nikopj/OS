#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fifo.h"

#define N_ITER 1e4
#define MEM_SIZE 0x10000

int main(int argc, char **argv)
{
	int i;
	unsigned long d;
	struct fifo *f;

	if( (f=mmap(NULL,MEM_SIZE,PROT_READ|PROT_WRITE,\
		MAP_SHARED|MAP_ANONYMOUS,-1,0))==MAP_FAILED )
	{
		perror("mmap error");
		exit(-1);
	}

	fifo_init(f);
	
	switch( fork() )
	{
		case -1:
			perror("fork error");
			exit(-1);
		case 0:
			for(d=0;d<N_ITER;d++)
				fifo_wr(f,d);
			exit(0);
		default:
			for(i=0;i<N_ITER;i++)
			{
				d = fifo_rd(f);
				printf("d: %lu\n",d);
			}
	}
	while(wait(NULL)>0);
	return 0;
}
