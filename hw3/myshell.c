// myshell.c
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
// for wait3()
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFF_SIZE 2048
#define DEBUG 0

void eprint_tokens(const char *msg, char **tokv);
int io_handle(char **tokv);
int io_redir(const char *filename, const int rfd, int flags, mode_t mode);

int main(int argc, char **argv)
{
	pid_t pid;
	int wstatus, exstat, tcheck;
	struct timeval tstart, tstop; time_t elps_ts; suseconds_t elps_tu;
	char *delimiter = " \t\n";
	char *line;
	char *ptokes[BUFF_SIZE]; // pointers to tokens
	struct rusage usage;
	FILE *fstream;

	if( (line = malloc(BUFF_SIZE)) == NULL )
	{
		perror("line malloc failure");
		exit(-1);
	}
	
	// if no input file is given or it cannot be opened
	// set the input to stdin
	if( argv[1]==NULL || (fstream=fopen(argv[1],"r"))==NULL )
	{
		if( argv[1]!=NULL )
			perror("fopen input error");
		fstream = stdin;
	}

	while(1)
	{
		printf("$ ");

		if( fgets(line, BUFF_SIZE, fstream) != NULL )
		{
			// tokenize input
			int i;
			ptokes[i=0] = strtok(line, delimiter);
			while(ptokes[i]!=NULL)
			{
				ptokes[++i] = strtok(NULL, delimiter);
			}
			if( ptokes[0]==NULL )
				continue;

			// DEBUGGING
			if(DEBUG)
				eprint_tokens("+ cmd: ", ptokes);
			
			// parse tokens
			if( strcmp(ptokes[0],"exit") == 0 ) // EXIT
			{
				if( ptokes[1]!=NULL )
					exstat = strtol(ptokes[1], NULL, 10);
				free(line);
				exit(exstat);
			}
			else if( strncmp(ptokes[0],"#",1) == 0 ) // COMMENT
				continue;
			else if( strcmp(ptokes[0],"cd")==0 && ptokes[1]!=NULL ) // CD
			{
				if( chdir(ptokes[1]) == -1 )
					perror("cd");
				continue;
			}
			else if( strcmp(ptokes[0],"pwd")==0 && ptokes[1]==NULL ) // PWD 
			{
				char *cwd;
				if( (cwd = malloc(BUFF_SIZE)) == NULL )
				{
					perror("cwd malloc failure");
					continue;
				}
				else
				{
					if( (getcwd(cwd, BUFF_SIZE)) != NULL )
						printf("%s\n",cwd);
					else
						perror("getcwd error");
					free(cwd);
				}
			}
			else // FORK -> EXEC
			{
				if( (tcheck = gettimeofday(&tstart,NULL)) == -1 )
					perror("gettimeofday error @ start");
				switch( pid=fork() )
				{
				case -1:
					perror("fork error");
					break;

				case 0:
					// close input file
					if( fclose(fstream)!=0 )
					{
						perror("input filestream close in child failed");
						free(line);
						exit(-1);
					}
					if(DEBUG)
						eprint_tokens("+ child tokes before: ", ptokes);
					if( io_handle(ptokes)!=0 ) // IO REDIRECTION HANDLER
					{
						fprintf(stderr,"IO redirection failure\n");
						free(line);
						exit(-1);
					}
					if(DEBUG)
						eprint_tokens("+ child tokes after: ", ptokes);
					execvp(ptokes[0], ptokes);
					perror("exec failed");
					free(line);
					exit(-1);

				default:
					if(DEBUG)
						fprintf(stderr, "+ in parent, child pid is %d\n", pid);
					if( wait3(&wstatus, 0, &usage) == -1 )
						perror("wait3 error");
					else
					{
						if( tcheck==-1 || gettimeofday(&tstop,NULL)==-1 )
						{
							elps_ts=-1; elps_tu=0;
							perror("gettimeofday error @ stop, real time <- -1.0");
						}
						else
						{
							elps_ts = tstop.tv_sec - tstart.tv_sec;
							elps_tu = 1000000*tstop.tv_sec + tstop.tv_usec \
								- 1000000*tstart.tv_sec - tstart.tv_usec;
						}
						fprintf(stderr,"child process %d:\n\texit status: %d\n"
							"\t usr time: %ld.%06lds \n\t sys time: %ld.%06ld\n"
							"\treal time: %ld.%06lds\n", \
							pid, exstat=WEXITSTATUS(wstatus), \
							usage.ru_utime.tv_sec, usage.ru_utime.tv_usec, \
							usage.ru_stime.tv_sec, usage.ru_stime.tv_usec, \
							elps_ts, elps_tu);
					}
				}
			}
		}
		else if( errno !=0 ) 
			perror("fgets line");
		else printf("\n"); // continue to next line
	}
	free(line);
	return 0;
}
// prints tokens pointed to by tokv to stderr
// msg is printed before tokens
void eprint_tokens(const char *msg, char **tokv)
{
	int i=0;
	fprintf(stderr, "%s", msg);
	while(tokv[i]!=NULL)
		fprintf(stderr, "%s ", tokv[i++]);
	fprintf(stderr, "\n");
}

// switch cases for IO redirection
// calls io_redir() if necessary
int io_handle(char **tokv)
{
	int i=0;
	int ret=0;
	while(tokv[i]!=NULL)
	{
		if(tokv[i+1]!=NULL)
		{
			if(strcmp(tokv[i],"<")==0)
			{
				if(DEBUG)
					fprintf(stderr,"+ rd stdin to %s\n",tokv[i+1]);
				ret = ret + io_redir(tokv[i+1], STDIN_FILENO, O_RDONLY, 0444);
				tokv[i++]=NULL;
			}
			else if(strcmp(tokv[i],">")==0)
			{
				if(DEBUG)
					fprintf(stderr,"+ rd stdout to %s\n",tokv[i+1]);
				ret = ret + io_redir(tokv[i+1],STDOUT_FILENO,O_CREAT|O_TRUNC|O_WRONLY,0666);
				tokv[i++]=NULL;
			}
			else if(strcmp(tokv[i],"2>")==0)
			{
				if(DEBUG)
					fprintf(stderr,"+ rd stdder to %s\n",tokv[i+1]);
				ret = ret + io_redir(tokv[i+1],STDERR_FILENO,O_CREAT|O_TRUNC|O_WRONLY,0666);
				tokv[i++]=NULL;
			}
			else if(strcmp(tokv[i],">>")==0)
			{
				if(DEBUG)
					fprintf(stderr,"+ rd stout to %s\n",tokv[i+1]);
				ret = ret + io_redir(tokv[i+1],STDOUT_FILENO,O_CREAT|O_APPEND|O_WRONLY,0666);
				tokv[i++]=NULL;
			}
			else if(strcmp(tokv[i],"2>>")==0)
			{
				if(DEBUG)
					fprintf(stderr,"+ rd stdin to %s\n",tokv[i+1]);
				ret = ret + io_redir(tokv[i+1],STDERR_FILENO,O_CREAT|O_APPEND|O_WRONLY,0666);
				tokv[i++]=NULL;
			}
		}
		i++;
	}
	return ret;
}

// IO redirection
int io_redir(const char *filename, const int rfd, int flags, mode_t mode)
{
	int ofd;
	if( (ofd=open(filename, flags, mode)) < 0 )
	{
		fprintf(stderr,"%s open error: %s\n",filename,strerror(errno));
		return -1;
	}
	else if( dup2(ofd,rfd)!=-1 )
	{
		close(ofd);
		return 0;
	}
	else 
		fprintf(stderr,"%s to fd=%d, dup2 error: %s\n",filename,rfd,strerror(errno));
	return -1;
}
