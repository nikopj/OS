// myshell.c
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	char *delimiter = " \t\n";
	char *line;
	char *ptokes[1024]; // stores pointers to tokens
	if( !(line = malloc(4069)) )
	{
		perror("line malloc failure");
		exit(-1);
	}

	while(1)
	{
		printf("$ ");

		if( fgets(line, 4096, stdin) != NULL )
		{

			int i;
			ptokes[i=0] = strtok(line, delimiter);
			while(ptokes[i]!=NULL)
			{
				ptokes[++i] = strtok(NULL, delimiter);
			}

			if( strcmp(ptokes[0],"exit") == 0 ) // EXIT
				break;
			else if( strncmp(ptokes[0],"#",1) == 0 ) // COMMENT
				continue;
			else // CMD
			{
				if( strcmp(ptokes[0],"cd")==0 && ptokes[1]!=NULL)
				{
					if( !(chdir( ptokes[1] )) )
						perror("cd");
				}

				fprintf(stderr, "+ cmd: ");
				i=0;
				while(ptokes[i]!=NULL)
					fprintf(stderr, "%s ", ptokes[i++]);
				fprintf(stderr, "\n");
			}
		} 
		else 
			perror("fgets line");
	}

	free(line);
	return 0;
}
