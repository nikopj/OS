#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sem.h"

#define N_PROC 64
#define N_ITER 100
#define MEM_SIZE 4096
#define DEBUG 1

int main(int argc, char **argv)
{
	int i,p,wstatus;
	pid_t pid;
	struct sem *s;

	if( (s=mmap(NULL,MEM_SIZE,PROT_READ|PROT_WRITE,\
		MAP_SHARED|MAP_ANONYMOUS,-1,0))==MAP_FAILED )
	{
		perror("mmap error");
		exit(-1);
	}

	sem_init(s,1);

	for(i=0;i<N_PROC;i++)
	{
		switch( pid=fork() )
		{
			case -1:
				fprintf(stderr,"i=%d fork error: %s\n",i,strerror(errno));
				exit(-1);
			case 0:
				if(i%2==0)
				{
					for(i=0;i<N_ITER;i++)
						sem_wait(s);
				}
				else
				{
					for(i=0;i<N_ITER;i++)
						sem_inc(s);
				}
				exit(0);
		}
	}
	while( (p=wait(&wstatus))>0 || errno==EINTR );
	printf("count=%d\n",s->count);
		
	return 0;
}
