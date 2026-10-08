#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(int argc, char *argv[])
{
  int i, j;
  int n = 10; 
  int pid;
  int priorities[10] = {5, 10, 1, 7, 3, 9, 2, 6, 4, 8};
  int child_pids[10];

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
    
    // Parent stores the child PID
    child_pids[i] = pid;
  }
  printf("Parent: Waiting for children to finish...\n\n");
  
  int finished_pid;
  for(i = 0; i < n; i++){
    finished_pid = wait(0);
    
    if(finished_pid > 0){
        // Find the priority for this PID
        int priority = -1;
        for(j = 0; j < n; j++){
            if(child_pids[j] == finished_pid){
                priority = priorities[j];
                break;
            }
        }
        printf(">>> Child with PID %d (priority %d) has finished.\n", finished_pid, priority);
    }
  }

  printf("\nStress test finished.\n");
  exit(0);
}