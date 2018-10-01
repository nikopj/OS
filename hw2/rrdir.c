/*
Nikola Janjusevic 2018
rrdir.c -- recursively traverses directory, printing out meta data stats
*/

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define BUFFSIZE 256

// recursively read directories (and print stats)
int rrdir(char *dn);

int main(int argc, char **argv)
{
  char *path;
  if(argc == 2)
  {
    path = argv[1];
    // remove any leading slashes for nicer printing
    while(path[strlen(path)-1] == '/')
      path[strlen(path)-1] = '\0';
    rrdir(argv[1]);
  }
  else
    printf("Usage: %s [staring directory]\n", argv[0]);
  return 0;
}

// returns char corresponding to filetype given by st_mode from stat struct
char charftype(mode_t st_mode)
{
  switch( st_mode & S_IFMT )
  {
    case S_IFSOCK: return 's';
    case S_IFLNK:  return 'l';
    case S_IFREG:  return '-';
    case S_IFBLK:  return 'b';
    case S_IFDIR:  return 'd';
    case S_IFCHR:  return 'c';
    case S_IFIFO:  return 'p';
    default:       return '?';
  }
}

// returns string of file permission in rwx format (puts in dest)
// (for user, group, and others)
char *strfperm(mode_t st_mode, char *dest)
{
  char octperm[4];
  char *cptr;
  sprintf(octperm, "%03o", st_mode & ~S_IFMT);

  int i;
  for(i=0; i<3; i++)
  {
    cptr = dest + 3*i;
    switch(octperm[i])
    {
      case '0': strcpy(cptr, "---");
        break;
      case '1': strcpy(cptr, "--x");
        break;
      case '2': strcpy(cptr, "-w-");
        break;
      case '4': strcpy(cptr, "r--");
        break;
      case '5': strcpy(cptr, "r-x");
        break;
      case '6': strcpy(cptr, "rw-");
        break;
      case '7': strcpy(cptr, "rwx");
        break;
      default:  strcpy(cptr, "???");
        break;
    }
  }
  return dest;
}

// puts name corresponding to uid into dest string.
// if correspondence found, uid number is put into dest string.
// -1 returned upon error, 0 upon success
int strfuid(uid_t st_uid, char *dest)
{
  struct passwd *stpw;
  errno = 0; // reset errno first as per readdir's man page suggestion
  if( !(stpw = getpwuid(st_uid)) )
  {
    if( errno == 0 )  // no uid - name translation
    {
      snprintf(dest, BUFFSIZE, "%d", st_uid);
      return 0;
    }
    else
    {
      fprintf(stderr, "Error getting UID name from %d:%s\n", st_uid, \
        strerror(errno));
    }
    return -1;
  }
  else
    snprintf(dest, BUFFSIZE, "%s", stpw->pw_name);
  return 0;
}

// same as strfuid except with gid
int strfgid(uid_t st_gid, char *dest)
{
  struct group *stgr;
  errno = 0; // reset errno first as per readdir's man page suggestion
  if( !(stgr = getgrgid(st_gid)) )
  {
    if( errno == 0 )  // no uid - name translation
    {
      snprintf(dest, BUFFSIZE, "%d", st_gid);
    }
    else
    {
      fprintf(stderr, "Error getting UID name from %d:%s\n", st_gid, \
        strerror(errno));
    }
    return -1;
  }
  else
    snprintf(dest, BUFFSIZE, "%s", stgr->gr_name);
  return 0;
}

char *strflnk(char *path, char *dest)
{
  ssize_t len;
  if( !(len = readlink(path, dest, PATH_MAX)) )
    fprintf(stderr, "Error reading link %s:%s\n", path, strerror(errno));
  else
  {
    dest[len] = '\0';
  }
  return dest;
}

// prints path meta data to standard output
void printstats(char *path, struct stat *st)
{
  char strperm[10];
  char struid[BUFFSIZE];
  char strgid[BUFFSIZE];
  char strmtime[13];
  char ft;
  char strlnk[PATH_MAX];
  char *lnksymb;

  // get filetype character
  if( (ft = charftype(st->st_mode)) == 'l' )
  { // get symbolic link path if it exists
    strflnk(path, strlnk);
    lnksymb = "-> ";
  }
  else
  { // otherwise make sure these don't print anything
    *strlnk  = '\0';
    lnksymb = "";
  }

  // get permissions string
  strfperm(st->st_mode, strperm);

  // get uid and gid names
  strfuid(st->st_uid, struid);
  strfgid(st->st_gid, strgid);

  // get time string
  struct tm *tmp = localtime(&(st->st_mtime));
  strftime(strmtime, sizeof(strmtime), "%b %e %R", tmp);

  printf(" %8ld %6ld %c%9s %3ld %-9s %-9s %8ld %s %s %s%s\n", \
    st->st_ino, st->st_blocks/2, ft, strperm, st->st_nlink, struid, strgid, \
    st->st_size, strmtime, path, lnksymb, strlnk);
}

// recursively read directory
int rrdir(char *dn)
{
  DIR *dirp;
  struct dirent *de;
  char *path;

  // initialize path for this fcn call
  if( !(path = malloc(strlen(dn)+1)) )
  {
    fprintf(stderr, "Error in path malloc: %s\n", strerror(errno));
    exit(-1);
  }

  // open directory before attempting to read
  if( !(dirp=opendir(dn)) )
  {
    fprintf(stderr, "Cannot open directory %s: %s\n", dn, strerror(errno) );
    return -1;
  }

  errno = 0; // reset errno first as per readdir's man page suggestion
  while( de=readdir(dirp) ) // loop through directory entries
  {
    struct stat st;

    // allocate memory for pathname
    if ( !(path = realloc(path, strlen(dn) + strlen(de->d_name) + 2)) )
    {
      fprintf(stderr, "Error in path realloc: %s\n", strerror(errno));
      exit(-1);
    }
    // append directory entry to path
    sprintf(path, "%s/%s", dn, de->d_name);

    // if de refers to . or .., don't print stats
    if( strcmp(de->d_name, "..") == 0 || strcmp(de->d_name, ".") == 0 )
      continue;

    // get file meta data
    if( lstat(path, &st) == -1 )
    {
      fprintf(stderr, "Error getting stats from %s: %s\n", path, \
        strerror(errno));
      continue;
    }

    // print meta data to stdout
    printstats(path, &st);

    // recursively explore child directories
    if( S_ISDIR(st.st_mode) )
      rrdir(path);
    errno = 0;
  }

  if(errno)
    fprintf(stderr, "Error reading directory %s: %s\n", dn, strerror(errno));
  if( closedir(dirp) == -1 )
    fprintf(stderr, "Error closing directory %s: %s\n", dn, strerror(errno));

  free(path);
}
