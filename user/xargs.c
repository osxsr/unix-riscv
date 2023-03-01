#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAXARGS 10

int
main(int argc, char *argv[])
{
  char *newargv[MAXARGS];
  int newargc = 0, fk;
  char buf[256];
  int i=0, n=0, lastn=0;

  if(argc <= 1){
    fprintf(2, "xargs usage: executable file argv...\n");
    exit(1);
  }

  for(newargc = 0; newargc < argc-1; newargc++)
    newargv[newargc] = argv[newargc + 1];
  while((n=read(0, buf+lastn, sizeof(buf))+lastn) > 0) {
    lastn = 0;
    for(i=0; i<n; i++) {
      if(buf[i] == '\n'){
        newargv[newargc] = malloc((i-lastn)*sizeof(char));
        memcpy(newargv[newargc], buf+lastn, i-lastn);
        newargc++;
        lastn = i+1;
        if(newargc >= MAXARGS) {
          fprintf(2, "ERROR :xargs too many argvs!\n");
          exit(1);
        }
      }
    }
    if(lastn == 0&&n == sizeof(buf)) {
      fprintf(2, "ERROR : argv%d is too long!\n", newargc);
      exit(1);
    }
    memmove(buf, buf + lastn, n - lastn);
    lastn = n - lastn; 
  }

  fk = fork();
  if(fk < 0) {
    fprintf(2, "xargs: fork failed\n");
    exit(2);
  }
  else if( fk == 0 )
    exec(newargv[0], newargv);
  else 
    wait(0);
    
  for(i= argc-1; i<newargc; i++)
    free(newargv[i]);
  exit(0);
}
