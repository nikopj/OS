# include <fcntl.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>

# define BUFFSIZE 8

int main(int argc, char **argv)
{
  int i, n, infile_start, fd_rd;
  int fd_wr = STDOUT_FILENO;
  char *outfile = "stdout";
  int buff_size = BUFFSIZE;

  // argument parsing
  for(i=1; i<argc; i++)
  {
    // set buffer size
    if(i<(argc-1) && (strcmp(argv[i], "-b") == 0))
    {
      if((buff_size = atoi(argv[++i])) < 1)
      {
        fprintf(stderr,"buffer size error: invalid input\n");
        exit(-1);
      }
    }
    // get output file descriptor
    if(i<(argc-1) && (strcmp(argv[i], "-o") == 0))
    {
      if((fd_wr = open(argv[++i], O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU)) == -1)
      {
        fprintf(stderr,"outfile \"%s\" open error: %s", \
          argv[i],strerror(errno));
        exit(-1);
      }
      outfile = argv[i];
      printf("%s\n", outfile);
    }
    // stop loop at first infile
    if(strncmp(argv[i-1], "-", 1) != 0)
      break;
  }
  char buf[buff_size];
  printf("i:%d\n",i);
  // if no infiles provided
  if(i>argc-1)
  {
    printf("here1\n");
    while((n = read(STDIN_FILENO, buf, buff_size)) > 0)
    {
      if(write(fd_wr, buf, n) != n)
      {
        fprintf(stderr,"infile \"%s\" write error: %s", \
          argv[i],strerror(errno));
        exit(-1);
      }
    }
  }
  else // some infiles provided
  {
    printf("here2\n");
    while(i<argc)
    {
      if(strncmp(argv[i], "-", 1) == 0)
        fd_rd = STDIN_FILENO;
      else if(fd_rd = open(argv[i], O_RDONLY) == -1)
      {
        fprintf(stderr,"infile \"%s\" open error: %s", \
          argv[i],strerror(errno));
        exit(-1);
      }
      while((n = read(fd_rd, buf, buff_size)) > 0)
      {
        if(write(fd_wr, buf, n) != n)
        {
          fprintf(stderr,"outfile \"%s\" open error: %s", \
            outfile,strerror(errno));
          exit(-1);
        }
      }
      if(n < 0)
      {
        fprintf(stderr,"infile \"%s\" read error: %s", \
          argv[i],strerror(errno));
        exit(-1);
      }
      i++;
    }
  }

  return 0;
}
