/* 
Nikola Janjusevic, OS pset5
smear TARGET REPLACEMENT file1 {file2 ...}
*/
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define DEBUG 0

int main(int argc, char **argv)
{
	int i, fd;
	char *aptr, *itr;
	struct stat stat;

	// ensure proper arguments given
	if(argc<4)
	{
		fprintf(stderr,"usage: smear TARGET REPLACEMENT file1 {file2 ...}\n");
		return 1;
	}
	if( strlen(argv[1])!=strlen(argv[2]) )
	{
		fprintf(stderr,"usage: TARGET must be same length as REPLACEMENT\n");
		return 1;
	}

	for(i=3; i<argc; i++)
	{
		if(DEBUG)
			fprintf(stderr,"+ infile: %s\n",argv[i]);
		// open file
		if( (fd=open(argv[i],O_RDWR))<0 )
		{
			fprintf(stderr,"infile %s open error: %s\n", argv[i], \
				strerror(errno));
			return -1;
		}
		// fstat file to find size in bytes
		if( fstat(fd, &stat)<0 )
		{
			fprintf(stderr,"infile %s fstat error: %s\n", argv[i], \
				strerror(errno));
			// may want to continue to next file...
			return -1;
		}
		
		// map file into memory
		if( (aptr = mmap(NULL,stat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) \
			== MAP_FAILED)
		{
			fprintf(stderr,"infile %s mmap error: %s\n", argv[i], \
				strerror(errno));
			return -1;
		}

		// search and replace
		itr = aptr;
		while(itr-aptr < stat.st_size-strlen(argv[1]))
		{
			if(strncmp(itr,argv[1],strlen(argv[1]))==0)
				memcpy(itr,argv[2],strlen(argv[1]));
			itr++;
		}

		// write map
		if( msync(aptr,stat.st_size,MS_SYNC)<0 )
		{
			fprintf(stderr,"infile %s msync error: %s\n", argv[i], \
				strerror(errno));
			return -1;
		}

		// unmap
		if( munmap(aptr,stat.st_size)<0 )
		{
			fprintf(stderr,"infile %s munmap error: %s\n", argv[i], \
				strerror(errno));
			return -1;
		}

		// close file
		if( close(fd)<0 ){
			fprintf(stderr,"infile %s close error: %s\n", argv[i], \
				strerror(errno));
			return -1;
		}
	}

	return 0;
}
