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
# include <sys/wait.h>

# define DEBUG 1
# define BUFFSIZE 4069

pid_t execPipeIO(const char *cmd, const char *arg, int ifd, int ofd, \
	const int *close_fds, const int n);
void cat(int fdi, int fdo, char *buf, int buff_size);

int main(int argc, char **argv)
{
	pid_t pid1, pid2;
	int i, fd_in;
	int pipes[4];
	int bs = BUFFSIZE;
	char *buf;
	int wstatus;

	// argument parsing
	if( argc<3 )
	{
		fprintf(stderr,"%s: must specify grep pattern and infile\n",argv[0]);
		exit(-1);
	}

	// buffer allocation
	if( (buf=malloc(bs)) < 0 )
	{
		perror("malloc error");
		exit(-1);
	}


	// loop over infiles
	for(i=2; i<argc; i++)
	{
		// open infile
		if( (fd_in=open(argv[i], O_RDONLY)) < 0)
		{
			fprintf(stderr,"infile %s open error: %s\n", \
				argv[i], strerror(errno));
			exit(-1);
		}
		
		// pipe creation
		if(pipe(pipes)<0 || pipe(pipes+2)<0)
		{
			perror("error creating pipes");
			exit(-1);
		}
		// pipe[0] grep read
		// pipe[1] cat write
		// pipe[2] more read
		// pipe[3] grep write
		if(DEBUG)
		{
			fprintf(stderr,"+ pipes[]={%d,%d,%d,%d}\n", \
				pipes[0],pipes[1],pipes[2],pipes[3]);
		}

		// FORK -> EXEC GREP
		// array of file descriptors for child process to close
		const int close_fds1[3] = {pipes[1],pipes[2],fd_in};
		pid1 = execPipeIO("grep",argv[1],pipes[0],STDOUT_FILENO,close_fds1,3);
		if(DEBUG)
			fprintf(stderr,"+ grep pid: %d\n",pid1);

		// FORK -> EXEC MORE
		const int close_fds2[4] = {pipes[0],pipes[1],pipes[3],fd_in};
		pid2 = execPipeIO("more",NULL,pipes[2],STDOUT_FILENO,close_fds2,4);
		if(DEBUG)
			fprintf(stderr,"+ more pid: %d\n",pid2);

		// close pipe references
		if( close(pipes[0])==-1 || close(pipes[2])==-1 || close(pipes[3])==-1 )
			perror("error, parent couldn't close pipe references");

		fprintf(stderr,"+ catting infile %s\n",argv[i]);
		// actual concatenation
		cat(fd_in, pipes[1], buf, bs);

		// close the infile
		if(fd_in != STDIN_FILENO && close(fd_in) < 0) 
		{
			fprintf(stderr, "infile %s close error: %s\n", \
				argv[i], strerror(errno));
			exit(-1);
		}
		
		if( close(pipes[1])<0 )
		{
			fprintf(stderr,"cat write pipe close error: %s\n",strerror(errno));
			exit(-1);
		}

		// wait for ALL child processes to terminate
		int p;
		while( (p=wait(&wstatus))>0 )
		{
			if(DEBUG)
				fprintf(stderr,"+ terminated pid: %d\n",p);
		}
	}


	free(buf);
	return 0;
}

// pipeIO redirection and execution of cmd
// uses execlp with only one argument, arg
pid_t execPipeIO(const char *cmd, const char *arg, int ifd, int ofd, \
	const int *close_fds, const int n)
{
	pid_t pid;
	int i;
	switch( pid=fork() )
	{
	case -1:
		fprintf(stderr,"%s fork error: %s\n", cmd, strerror(errno));
		return -1;
	case 0:

		// redirect IO to pipes
		if( dup2(ifd,STDIN_FILENO)==-1 )
		{
			fprintf(stderr,"%s in-pipe failure: %s\n", cmd, strerror(errno));
			exit(-1);
		}
		if( dup2(ofd,STDOUT_FILENO)==-1 )
		{
			fprintf(stderr,"%s out-pipe failure: %s\n", cmd, strerror(errno));
			exit(-1);
		}

		// close dangling files
		for(i=0; i<n; i++)
		{
			if( close(close_fds[i])==-1 )
			{
				perror("error closing dangling fds in child");
				exit(-1);
			}
		}

		// execute
		execlp(cmd,cmd,arg,NULL);
		fprintf(stderr,"%s cmd exec failed: %s\n", cmd, strerror(errno));
		exit(-1);

	default:
		return pid;
	}		
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

