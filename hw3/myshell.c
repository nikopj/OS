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
// for ftime()
#include <sys/timeb.h>

#define BUFF_SIZE 2048

void eprint_tokens(const char *msg, char **tokv);

int main(int argc, char **argv)
{
	pid_t pid;
	int wstatus, exstat, tcheck;
	struct timeb tstart, tstop;
	time_t elps_ts;
	unsigned short elps_tm;
	char *delimiter = " \t\n";
	char *line;
	char *ptokes[BUFF_SIZE]; // pointers to tokens
	struct rusage usage;

	if( (line = malloc(BUFF_SIZE)) == NULL )
	{
		perror("line malloc failure");
		exit(-1);
	}

	while(1)
	{
		printf("$ ");

		if( fgets(line, BUFF_SIZE, stdin) != NULL )
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
			eprint_tokens("+ cmd: ", ptokes);
			
			// parse tokens
			if( strcmp(ptokes[0],"exit") == 0 ) // EXIT
			{
				if( ptokes[1]!=NULL )
					exstat = strtol(ptokes[1], NULL, 10);
				else
					exstat = 0;
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
				switch( pid=fork() )
				{
				case -1:
					perror("fork error");
					break;

				case 0:
					fprintf(stderr, "+ in child, cmd:");
					eprint_tokens(" ", ptokes);
					execvp(ptokes[0], ptokes);
					perror("exec failed");
					free(line);
					exit(-1);

				default:
					fprintf(stderr, "+ in parent, child pid is %d\n", pid);
					if( (tcheck = ftime(&tstart)) == -1 )
						perror("ftime error @ start");
					if( wait3(&wstatus, 0, &usage) == -1 )
						perror("wait3 error");
					else
					{
						if( tcheck==-1 || ftime(&tstop)==-1 )
						{
							elps_ts=-1; elps_tm=0;
							perror("ftime error @ stop, real time <- -1.0");
						}
						else
						{
							elps_ts = tstop.time - tstart.time;
							elps_tm = tstop.millitm - tstart.millitm;
						}
						fprintf(stderr, "+ child process %d:\n\texit status: %d\n"
							"\tusr time: %ld.%.6lds \n\tsys time: %ld.%.6lds \n"
							"\treal time: %ld.%.6ds\n", \
							pid, WEXITSTATUS(wstatus), usage.ru_utime.tv_sec, \
							usage.ru_utime.tv_usec, usage.ru_stime.tv_sec,   \
							usage.ru_stime.tv_usec, elps_ts, elps_tm);
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
