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
	char *outfile = "";
	char *buf;
	char c;

	// argument parsing
	while( (c=getopt(argc,argv,"b:o:"))!=-1 )
	{
		switch(c)
		{
		 case 'b':
			if( (bs = atoi(optarg)) < 1 )
			perror("invalid buffer-size");
			break;
		 case 'o':
			outfile = optarg;
			if( (fd_out = open(outfile, O_CREAT|O_WRONLY|O_TRUNC, 0666)) < 0)
			{
			fprintf(stderr, "outfile %s open error: %s\n", \
				outfile, strerror(errno));
			exit(-1);
			}
			break;
		}
	}
	// buffer allocation
	if( (buf=malloc(bs)) < 0 )
	{
		perror("malloc error");
		exit(-1);
	}
	// if no infiles given
	if(optind == argc)
		cat(STDIN_FILENO, STDOUT_FILENO, buf, bs);
	else // infiles given
	{
		// loop over infiles
		for(i=optind; i<argc; i++)
		{
			// hyphen specifies stdin
			if(strcmp(argv[i],"-") == 0)
				fd_in = STDIN_FILENO;
			else if( (fd_in=open(argv[i], O_RDONLY)) < 0)
			{
				fprintf(stderr,"infile %s open error: %s\n", \
					argv[i], strerror(errno));
				exit(-1);
			}
			if(strcmp(argv[i], outfile) == 0)
			{
				fprintf(stderr, "error: infile %s is also outfile\n",outfile);
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
	}

	if(fd_out != STDOUT_FILENO && close(fd_out) < 0)
	{
		fprintf(stderr, "outfile %s close error: %s\n", \
			outfile, strerror(errno));
		exit(-1);
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

