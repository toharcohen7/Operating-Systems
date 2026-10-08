#include "kernel/types.h"
#include "user/user.h"

// Test 1: Resource Exhaustion (Leak Check)
// We try to create and free semaphores more times than MAX_SEMS.
// If free() is broken, this will fail eventually.
void
test_resource_leaks()
{
  int i, sem;
  int limit = 300; // Assuming MAX_SEMS is 128, this forces reuse
  
  printf("\n=== TEST 4: Resource Leak / Reuse Check ===\n");

  for(i = 0; i < limit; i++){
    sem = sem_create(1);
    if(sem < 0){
      printf("Failed to create semaphore at iteration %d\n", i);
      printf("It seems sem_free is not clearing the 'used' flag properly.\n");
      exit(1);
    }
    
    // Use it briefly
    sem_down(sem);
    sem_up(sem);

    if(sem_free(sem) < 0){
      printf("Failed to free semaphore %d\n", sem);
      exit(1);
    }
  }
  printf("Successfully created and freed %d semaphores.\n", limit);
  printf("=== TEST 4 PASSED ===\n");
}

// Test 2: Producer-Consumer Simulation
// Uses two semaphores to coordinate "empty" and "full" slots.
void
test_producer_consumer()
{
  int sem_empty, sem_full;
  int pid;
  int transactions = 10;

  printf("\n=== TEST 5: Producer-Consumer Logic ===\n");

  // sem_empty: Starts with 1 (buffer has space)
  sem_empty = sem_create(1);
  // sem_full: Starts with 0 (buffer has no data)
  sem_full = sem_create(0);

  if(sem_empty < 0 || sem_full < 0) {
      printf("Failed to create semaphores\n");
      exit(1);
  }

  pid = fork();

  if(pid == 0) {
    // --- CONSUMER (Child) ---
    for(int i = 0; i < transactions; i++) {
      // Wait for data (down on full)
      sem_down(sem_full);
      printf("Consumer: Got item %d\n", i);
      // Signal empty slot (up on empty)
      sem_up(sem_empty);
    }
    exit(0);
  } else {
    // --- PRODUCER (Parent) ---
    for(int i = 0; i < transactions; i++) {
      // Wait for space (down on empty)
      sem_down(sem_empty);
      printf("Producer: Sent item %d\n", i);
      // Signal data ready (up on full)
      sem_up(sem_full);
    }
    wait(0); // Wait for child
  }

  sem_free(sem_empty);
  sem_free(sem_full);
  printf("=== TEST 5 PASSED ===\n");
}

int
main(int argc, char *argv[])
{
  printf("Starting ADVANCED tests...\n");
  
  test_resource_leaks();
  test_producer_consumer();

  printf("\nALL ADVANCED TESTS PASSED.\n");
  exit(0);
}