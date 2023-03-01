#include"user/user.h"

int 
main(int argc, char *argv[])
{
  int num;

  if(argc < 2){
    fprintf(2, "Usage: sleep number of ticks...\n");
    exit(1);
  }

  num = atoi(argv[1]);
  sleep(num);

  exit(0);
}