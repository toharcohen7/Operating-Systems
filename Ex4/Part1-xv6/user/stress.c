#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(int argc, char *argv[])
{
  int i;
  int n = 3; 
  int pid;
  int priorities[3] = {5, 10, 1}; 

  printf("\n--- Stress test: Launching %d instances of spin ---\n", n);

  for(i = 0; i < n; i++){
    pid = fork();
    
    if(pid < 0){
      printf("fork failed\n");
      exit(1);
    }
    
    if(pid == 0){
      if(set_ps_priority(priorities[i]) != 0){
        printf("Error, priority is larger than 10 or less than 1");
        exit(-1);
      } 
      
      printf("Debug: Child (PID %d) launched with priority %d\n", getpid(), priorities[i]);

      char *args[] = { "spin", 0 };
      exec("spin", args);
      
      printf("exec failed\n");
      exit(1);
    }
  }
  printf("Parent: Waiting for children to finish...\n\n");
  
  int finished_pid;
  for(i = 0; i < n; i++){
    finished_pid = wait(0);
    
    if(finished_pid > 0){
        printf(">>> Child with PID %d has finished.\n", finished_pid);
    }
  }

  printf("\nStress test finished.\n");
  exit(0);
}