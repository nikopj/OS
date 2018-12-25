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
#define N_PROC 16
#define MEM_SIZE 0x10000

int main(int argc, char **argv)
{
	int i,j,k, n_vp;
	unsigned long d=0;
	struct fifo *f;

	if( (f=mmap(NULL,MEM_SIZE,PROT_READ|PROT_WRITE,\
		MAP_SHARED|MAP_ANONYMOUS,-1,0))==MAP_FAILED )
	{
		perror("mmap error");
		exit(-1);
	}

	fifo_init(f);
	
	for(i=0;i<N_PROC;i++)
	{
		switch( fork() )
		{
			case -1:
				perror("fork error");
				exit(-1);
			case 0:
				for(j=0;j<N_ITER;j++)
				{	
					d = (unsigned long)i<<32 | (unsigned long)j;
					fifo_wr(f,d);
				}
				exit(0);
			default:
				break;
		}
	}
	for(j=0;j<N_PROC*N_ITER;j++)
	{
		d = fifo_rd(f);
		n_vp = (int)(d>>32);
		k = (int)d;
		printf("n_vp: %d,\t k: %d\n",n_vp,k);
	}
	while(wait(NULL)>0);
	return 0;
}
