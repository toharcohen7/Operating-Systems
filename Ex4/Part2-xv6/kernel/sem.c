// kernel/sem.c
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

struct sem {
  struct spinlock lock;  // Protects this semaphore's value
  int value;             // Number of resources available
  int used;              // Is this slot currently allocated?
};
struct sem sems[MAX_SEMS];
struct spinlock sem_array_lock; // Protects allocation/deallocation

// Initialize the semaphore array
void
seminit(void)
{
  int i;

  initlock(&sem_array_lock, "sem_array_lock");
  
  for(i = 0; i < MAX_SEMS; i++) {
    initlock(&sems[i].lock, "semaphore");
    sems[i].used = 0;
  }
}

// Create a new semaphore with the given initial value
int
sem_create(int init_value)
{
  int i;
  
  if(init_value < 0)
    return -1;

  // Lock the array to find a slot safely
  acquire(&sem_array_lock);

  // Find a free semaphore slot
  for(i = 0; i < MAX_SEMS; i++) {
    if(sems[i].used == 0) {
      sems[i].used = 1;
      sems[i].value = init_value;
      release(&sem_array_lock);
      return i; 
    }
  }

  release(&sem_array_lock);
  return -1; // No free slots available
}

// Free the semaphore at the given index
int
sem_free(int index)
{
  struct sem *s;

  // Validate index
  if(index < 0 || index >= MAX_SEMS)
    return -1;

  s = &sems[index];

  // Lock the array and the semaphore to free it safely
  acquire(&sem_array_lock);
  acquire(&s->lock);

  // Check if it's already free
  if(s->used == 0) {
    release(&s->lock);
    return -1;
  }
  
  // Mark as free
  s->used = 0;

  // Release locks
  release(&s->lock);
  release(&sem_array_lock);
  
  return 0;
}

// 
int
sem_down(int id)
{
  struct sem *s;

  // Validate id
  if(id < 0 || id >= MAX_SEMS)
    return -1;

  s = &sems[id];

  // Acquire the semaphore lock
  acquire(&s->lock);
  
  if(s->used == 0) {
    release(&s->lock);
    return -1;
  }

  // Wait until the semaphore value is positive
  while(s->value == 0) {
    sleep(s, &s->lock); 
  }

  s->value--;
  release(&s->lock);
  
  return 0;
}

// Up the semaphore at the given id
int
sem_up(int id)
{
  struct sem *s;

  //
  if(id < 0 || id >= MAX_SEMS)
    return -1;

  s = &sems[id];

  // Acquire the semaphore lock
  acquire(&s->lock);
  
  // Check if the semaphore is currently used
  if(s->used == 0) {
    release(&s->lock);
    return -1;
  }

  // Wake up all processes sleeping on this semaphore
  s->value++;
  wakeup(s); 
  
  release(&s->lock);
  return 0;
}

// System call handlers

uint64
sys_sem_create(void)
{
  int init_val;
  argint(0, &init_val);
  return sem_create(init_val);
}

uint64
sys_sem_free(void)
{
  int index;
  argint(0, &index);
  return sem_free(index);
}

uint64
sys_sem_down(void)
{
  int index;
  argint(0, &index);
  return sem_down(index);
}

uint64
sys_sem_up(void)
{
  int index;
  argint(0, &index);
  return sem_up(index);
}
