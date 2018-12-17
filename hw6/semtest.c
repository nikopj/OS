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
#define N_ITER 10
#define MEM_SIZE 4096
#define DEBUG 0

int main(int argc, char **argv)
{
	int i,p, wstatus;
	int n_vp = 0;
	char *mptr;
	pid_t pid;
	struct sem *s;

	if( (s=mmap(NULL,MEM_SIZE,PROT_READ|PROT_WRITE,\
		MAP_SHARED|MAP_ANONYMOUS,-1,0))==MAP_FAILED )
	{
		perror("mmap error");
		exit(-1);
	}

	for(i=0;i<N_PROC;i++)
	{
		switch( pid=fork() )
		{
			case -1:
				fprintf(stderr,"i=%d fork error: %s\n",i,strerror(errno));
				exit(-1);
			case 0:
				n_vp=i;
				if(n_vp%2==0)
				{
					for(i=0;i<N_ITER;i++)
						sem_inc(s);
				}
				else
				{
					for(i=0;i<N_ITER;i++)
						sem_wait(s,n_vp)
				}
				exit(0);
		}
	}
	while( (p=wait(&wstatus))>0 || errno==EINTR );
	printf("ideal : *x=%d\nactual: *x=%d\n",(int)(N_PROC*N_ITER),*x);
		
	return 0;
}
