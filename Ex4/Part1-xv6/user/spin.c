// user/spin.c
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;
  volatile int x = 0; 
  int count = 2000000000; 

  printf("Spinning... (pid %d)\n", getpid());

  for(i = 0; i < count; i++){
    x += 1;
  }

  printf("Done! (pid %d)\n", getpid());
  exit(0);
}