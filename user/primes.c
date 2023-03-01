#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p[2], n, fk, i, old;
  if(pipe(p)<0){
    fprintf(2, "panic: pipe\n");
    exit(1);
  }

  for(i=2; i<32; i++) {
    write(p[1], (char *)&i, 4);
  }
  close(p[1]);
  old = p[0];
  fk = fork();
  while(fk == 0){
    if(pipe(p)<0){
      fprintf(2, "panic: pipe\n");
      exit(1);
    }

    if(read(old, (char *)&n, 4)) {
      fprintf(2, "prime %d\n", n);
      while(read(old, (char *)&i, 4)) {
        if(i % n) {
          write(p[1], (char *)&i, 4);
        }
      }
    }
    else {
      exit(0);
    }

    close(old);
    close(p[1]);
    old = p[0];
    fk = fork();
  }

  wait(0);
  close(p[0]);
  exit(0);
}