/*
Nikola Janjusevic
Operating Systems Assignment 4
minicat.c -b -o infile
concatenates files with optional buffer size argument -b, and output file
argument -o. Files to cat are specified at the infile position.
Special character '-' (a single hyphen) may be used to allow for input from
the standard input.
*/
// argument parsing adapted from gnu.org example of getopt
# include <ctype.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <string.h>
# include <errno.h>

# define BUFFSIZE 4069

void cat(int fdi, int fdo, char *buf, int buff_size);

int main(int argc, char **argv)
{
	int i, fd_in;
	int bs = BUFFSIZE;
	int fd_out = STDOUT_FILENO;
	char *buf, *pat;

	// argument parsing
	if( argc<3 )
	{
		fprintf(stderr,"%s: must specify grep pattern and infile\n",argv[0]);
		exit(-1);
	}
	pat = argv[1];


	// buffer allocation
	if( (buf=malloc(bs)) < 0 )
	{
		perror("malloc error");
		exit(-1);
	}
	// loop over infiles
	for(i=2; i<argc; i++)
	{
		if( (fd_in=open(argv[i], O_RDONLY)) < 0)
		{
			fprintf(stderr,"infile %s open error: %s\n", \
				argv[i], strerror(errno));
			exit(-1);
		}

		// actual concatenation
		cat(fd_in, fd_out, buf, bs);

		// close the infile
		if(fd_in != STDIN_FILENO && close(fd_in) < 0) 
		{
			fprintf(stderr, "infile %s close error: %s\n", \
				argv[i], strerror(errno));
			exit(-1);
		}
	}

	free(buf);
	return 0;
}

// performs concatenation of infile to outfile
void cat(int fdi, int fdo, char *buf, int buff_size)
{
	int n,m;
	char *temp;
	while( (n=read(fdi, buf, buff_size)) > 0 )
	{
		temp = buf;
		while( (m = write(fdo, temp, n)) !=n )
		{
			fprintf(stderr,"outfile with fd=%d write error: %s\n", \
				fdo, strerror(errno));
			temp+=m;
		}
	}
	if(n<0)
	{
		fprintf(stderr,"infile with fd=%d read error: %s\n", \
			fdi, strerror(errno));
		exit(-1);
	}
}

