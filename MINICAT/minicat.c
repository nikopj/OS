/*
Nikola Janjusevic
Operating Systems Assignment 1
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

int main (int argc, char **argv)
{
  int i, n, fd_in;
  int buff_size = BUFFSIZE;
  int fd_out = STDOUT_FILENO;
  char *outfile = "";
  char *buf;
  char c;
  // argument parsing
  while( (c = getopt(argc, argv, "b:o:")) != -1)
  {
    switch(c)
    {
      case 'b':
        if( (buff_size = atoi(optarg)) < 1)
        {
          fprintf(stderr,"buffer size error: invalid input\n");
          exit(-1);
        }
        break;
      case 'o':
        outfile = optarg;
        if( (fd_out = open(outfile, O_CREAT|O_WRONLY|O_TRUNC, \
	  	    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)
        {
          fprintf(stderr,"outfile \"%s\" open error: %s\n", \
            outfile,strerror(errno));
          exit(-1);
        }
        break;
    }
  }
  // buffer allocation
  buf = malloc(buff_size);
  // if NO INFILES are given
  if(optind == argc)
  {
    while( (n = read(STDIN_FILENO, buf, buff_size)) > 0)
    {
      if(write(fd_out, buf, n) != n)
      {
        fprintf(stderr,"outfile \"%s\" write error: %s\n", \
          outfile,strerror(errno));
        exit(-1);
      }
    }
    if(n < 0)
    {
      fprintf(stderr, "infile [stdin] read error: %s\n", strerror(errno));
      exit(-1);
    }
  }
  else
  { // if INFILES ARE GIVEN
    for(i = optind; i < argc; i++)
    {
      if(strncmp(argv[i], "-", 1) == 0) // hyphen infile specifies stdin
        fd_in = STDIN_FILENO;
      else if( (fd_in = open(argv[i], O_RDONLY)) < 0)
      {
        fprintf(stderr,"infile \"%s\" open error: %s\n", \
          argv[i],strerror(errno));
        exit(-1);
      }
      if(strcmp(argv[i], outfile) == 0)
      {
		    fprintf(stderr,"error: infile \"%s\" is outfile\n", \
	  	    outfile);
        exit(-1);
      }
      while( (n = read(fd_in, buf, buff_size)) > 0)
      {
        if(write(fd_out, buf, n) != n)
        {
          fprintf(stderr,"outfile \"%s\" write error: %s\n", \
            outfile,strerror(errno));
          exit(-1);
        }
      }
      if(n < 0)
      {
        fprintf(stderr,"infile \"%s\" read error: %s\n", \
          argv[i],strerror(errno));
        exit(-1);
      }
      if(fd_in != STDIN_FILENO && close(fd_in) < 0) // close the infile
	    {
		    fprintf(stderr, "infile \"%s\" close error: %s\n", \
          argv[i], strerror(errno));
        exit(-1);
	    }
    }
  }
  if(fd_out != STDOUT_FILENO && close(fd_out) < 0) // close the outfile
  {
    fprintf(stderr, "outfile \"%s\" close error: %s\n", \
      outfile, strerror(errno));
    exit(-1);
  }
  free(buf);
  return 0;
}
