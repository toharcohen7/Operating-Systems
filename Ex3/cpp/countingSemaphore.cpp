#include "../hpp/countingSemaphore.hpp"

countingSemaphore::countingSemaphore(int initValue) : val(initValue) {
    
    // S1 is a mutex for protecting the counter
    sem_init(&s1, 0, 1);

    // S2 controls access based on resource availability
    int s2Init = (initValue > 0) ? 1 : 0;
    sem_init(&s2, 0, s2Init);
}

countingSemaphore::~countingSemaphore() {
    sem_destroy(&s1);
    sem_destroy(&s2);
}

void countingSemaphore::down() {
    // Wait on the delay semaphore and lock the counter
    sem_wait(&s2);
    sem_wait(&s1);

    val--;

    if (val > 0) {
        sem_post(&s2);
    }

    sem_post(&s1);
}

void countingSemaphore::up() {
    // Lock the counter
    sem_wait(&s1);
    
    val++;

    // If we went from 0 to 1, wake up waiters
    if (val == 1) {
        sem_post(&s2);
    }

    // Unlock the counter
    sem_post(&s1);
}

bool countingSemaphore::tryDown() {
    // Attempt to decrement the delay semaphore without blocking
    if (sem_trywait(&s2) != 0) {
        return false;
    }

    // Lock the counter
    sem_wait(&s1);
    
    val--;

    if (val > 0) {
        sem_post(&s2);
    }

    sem_post(&s1);
    return true;
}