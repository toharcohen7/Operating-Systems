#ifndef COUNTING_SEMAPHORE_HPP
#define COUNTING_SEMAPHORE_HPP

#include <semaphore.h>

class countingSemaphore {
private:
    sem_t s1;
    sem_t s2;
    int val;

public:
    // Constructor
    countingSemaphore(int initValue);

    // Destructor
    ~countingSemaphore();

    // Standard wait operation
    void down();

    // Standard signal operation
    void up();

    // Non-blocking wait, for the dispatcher
    bool tryDown();
};

#endif /* COUNTING_SEMAPHORE_HPP */
