#include "kernel/types.h"
#include "user/user.h"

void
test_zero_init()
{
  int pid, sem;
  printf("\n=== TEST 1: Init Value 0 (The Rendezvous) ===\n");
  
  // Create locked semaphore
  sem = sem_create(0);
  if(sem < 0) { printf("Error creating sem\n"); exit(1); }

  pid = fork();
  if(pid == 0) {
    // Child
    printf("Child: Trying to acquire sem(0)...\n");
    sem_down(sem); // Should block here
    printf("Child: Acquired! (This implies Parent signaled)\n");
    exit(0);
  }

  // Parent
  printf("Parent: Sleeping 20 ticks to ensure child blocks...\n");
  sleep(20); 
  printf("Parent: Signaling semaphore...\n");
  sem_up(sem); // Release child

  wait(0);
  sem_free(sem);
  printf("=== TEST 1 PASSED ===\n");
}

void
test_many_procs()
{
  int i, pid, sem;
  int procs = 10;
  
  printf("\n=== TEST 2: Thundering Herd (%d processes) ===\n", procs);
  
  // Only 1 concurrent process allowed
  sem = sem_create(1); 

  for(i=0; i < procs; i++) {
    pid = fork();
    if(pid == 0) {
      if(sem_down(sem) < 0) { printf("Fail down\n"); exit(1); }
      // Critical Section
      // printf is not atomic, so output might look messy, but functionality is what matters
      printf("Process %d entered CS.\n", getpid());
      sleep(5); // Hold it briefly
      if(sem_up(sem) < 0) { printf("Fail up\n"); exit(1); }
      exit(0);
    }
  }

  // Wait for all
  for(i=0; i < procs; i++) wait(0);
  
  sem_free(sem);
  printf("=== TEST 2 PASSED ===\n");
}

void
test_ping_pong()
{
  int pid, sem;
  int iterations = 100;

  printf("\n=== TEST 3: Rapid Acquire/Release (%d times) ===\n", iterations);
  
  sem = sem_create(1);

  pid = fork();
  if(pid == 0) {
    // Child
    for(int i=0; i < iterations; i++){
      sem_down(sem);
      // Critical section is empty for speed
      sem_up(sem);
    }
    exit(0);
  } 

  // Parent also runs
  for(int i=0; i < iterations; i++){
      sem_down(sem);
      sem_up(sem);
  }

  wait(0);
  sem_free(sem);
  printf("=== TEST 3 PASSED (No deadlock/panic) ===\n");
}

int
main(int argc, char *argv[])
{
  printf("Starting EDGE CASE tests...\n");
  
  test_zero_init();
  test_many_procs();
  test_ping_pong();

  printf("\nALL EDGE CASE TESTS PASSED.\n");
  exit(0);
}