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

#define BUFF_SIZE 2048

void eprint_tokens(const char *msg, char **tokv);

int main(int argc, char **argv)
{
	pid_t pid;
	int wstatus;
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
				break;
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
			else
			{
				switch( pid=fork() )
				{
				case -1:
					perror("fork error");
					free(line);
					exit(-1);

				case 0:
					fprintf(stderr, "+ in child, cmd:");
					eprint_tokens(" ", ptokes);
					execvp(ptokes[0], ptokes);
					perror("exec failed");
					free(line);
					exit(-1);

				default:
					fprintf(stderr, "+ in parent, child pid is %d\n", pid);
					if( wait3(&wstatus, 0, &usage) == -1 )
						perror("wait4 error");
					else
						fprintf(stderr, "+ child process %d:\n\texit status: %d\n"
							"\tusr time: %ld.%.6ld \n\tsys time: %ld.%.6ld \n", \
							pid, WEXITSTATUS(wstatus), usage.ru_utime.tv_sec, \
							usage.ru_utime.tv_usec, usage.ru_stime.tv_sec,   \
							usage.ru_stime.tv_usec);
				}
			}
		}
		else 
			perror("fgets line");
	}
	free(line);
	return 0;
}

void eprint_tokens(const char *msg, char **tokv)
{
	int i=0;
	fprintf(stderr, "%s", msg);
	while(tokv[i]!=NULL)
		fprintf(stderr, "%s ", tokv[i++]);
	fprintf(stderr, "\n");
}
