#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p[2],fk=-1;
  char c;

  if(pipe(p)<0){
    fprintf(2, "panic: pipe\n");
    exit(1);
  }

  write(p[1], "x", 1);

  fk = fork();
  if(fk == -1) {
    fprintf(2, "panic: fork\n");
    exit(1);
  }

  //fprintf(2, "a%da\n", fk);
  if( fk == 0 ) {
    read(p[0], &c, 1);
    fprintf(2, "%d: received ping\n", getpid());
    write(p[1], "y", 1);
    close(p[0]);
    close(p[1]);
    exit(0);
  }
  wait(0);
  read(p[0], &c, 1);
  fprintf(2, "%d: received pong\n", getpid());
  close(p[0]);
  close(p[1]);


  exit(0);
}