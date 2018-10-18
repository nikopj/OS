// myshell.c
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{

	while(1)
	{
		char *line;
		if( !(line = malloc(4069)) )
		{
			perror("line malloc failure");
			break;
		}

		printf("$ ");
		if( fgets(line, 4096, stdin) != NULL )
		{
			printf("read: %s\n", line);
			if( strcmp(line,"exit") == 0 )
				break;

			char *tok = strtok(line, " ");
			fprintf(stderr, "going into while\n");
			while(tok!=NULL)
			{
				printf("tok: %s\n", tok);
				tok = strtok(NULL, " ");
			}
			fprintf(stderr, "out of while\n");

		} 
		else 
			perror("scanf");

		free(line);
	}
	
	return 0;
}
