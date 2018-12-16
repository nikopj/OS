#include <stdio.h>
#include "spinlib.h"

#define N_PROC 64
#define MEM_SIZE 4096

int main(int argc, char **argv)
{
	char *aptr;
	if( (aptr=mmap(NULL,MEM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,	
	return 0;
}
