//  

#ifndef BOUNDED_BUFFER_HEADER
#define BOUNDED_BUFFER_HEADER

#include "countingSemaphore.hpp"
#include <queue>
#include <pthread.h>

using namespace std;

class boundedBuffer
{
private:
    queue<char *> q;
    int maxSize;

    pthread_mutex_t mutex;
    countingSemaphore* empty;
    countingSemaphore* full;

public:

    // Constructor
    boundedBuffer(int size);

    // Destructor
    ~boundedBuffer();

    // Insert an item into the queue
    void insert(char* s);

    // Remove an item from the queue
    char *remove();

    // Function for preventing blocking the dispatcher
    char *tryRemove();
};

#endif /* BOUNDED_BUFFER_HEADER */