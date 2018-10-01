#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

int main(int argc, char **argv)
{
  DIR *dirp;
  if(argc == 2)
  {
    if( !(dirp=opendir(argv[1])) )
    {
      fprintf(stderr, "Cannot open directory %s:%s\n", argv[1], strerror(errno) );
      return -1;
    }
    printf("%s\n",argv[1]);
  }
  else
    printf("Error, not enough input arguments\n");
  return 0;
}
