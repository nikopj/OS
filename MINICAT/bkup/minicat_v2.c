# include <fcntl.h>
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define BUFFSIZE 8

int main(int argc, char **argv)
{
  int i, n, infile_start, fd_rd;
  int fd_wr = STDOUT_FILENO;
  int buff_size = BUFFSIZE;

  int c;
  while ((c = getopt (argc, argv, "b:o:")) != -1)
    switch (c)
      {
      case 'b':
        aflag = 1;
        break;
      case 'o':
        bflag = 1;
        break;
      case '?':
        if (optopt == '-')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }

  printf("i:%d\n",i);
  printf("buff_size:%d\n",buff_size);
  char buf[buff_size];
  // if no infiles provided
  if(i>=argc-1)
  {
    while((n = read(STDIN_FILENO, buf, buff_size)) > 0)
    {
      if(write(fd_wr, buf, n) != n)
      {
        perror("outfile: write error");
        exit(1);
      }
    }
  }
  else
  {
    while(i<argc)
    {
      if(argv[i] == "-")
        fd_rd = STDIN_FILENO;
      else
        fd_rd = open(argv[i], O_RDONLY);
      while((n = read(fd_rd, buf, buff_size)) > 0)
      {
        if(write(fd_wr, buf, n) != n)
        {
          perror("outfile: write error");
          exit(1);
        }
      }
      if(n < 0)
      {
        printf("%s: ", argv[i]);
        perror("infile: read error");
        exit(1);
      }
      i++;
    }
  }

  return 0;
}
