#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spinlib.h"

#define N_PROC 64
#define N_ITER 1e6
#define MEM_SIZE 512
#define DEBUG 0

int main(int argc, char **argv)
{
	int i, *x, p, wstatus;
	char *mptr;
	volatile char *lock;
	pid_t pid;

	if( (mptr=mmap(NULL,MEM_SIZE,PROT_READ|PROT_WRITE,\
		MAP_SHARED|MAP_ANONYMOUS,-1,0))==MAP_FAILED )
	{
		perror("mmap error");
		exit(-1);
	}
	lock=mptr+10; *lock=0;
	x=(int*)mptr;

	for(i=0;i<N_PROC;i++)
	{
		switch( pid=fork() )
		{
			case -1:
				fprintf(stderr,"i=%d fork error: %s\n",i,strerror(errno));
				exit(-1);
			case 0:
				for(i=0;i<N_ITER;i++)
				{
					spin_lock(lock);
					(*x)++;
					spin_unlock(lock);
				}
				exit(0);
		}
	}
	while( (p=wait(&wstatus))>0 || errno==EINTR );
	printf("ideal : *x=%d\nactual: *x=%d\n",(int)(N_PROC*N_ITER),*x);
		
	return 0;
}
