//  

#include "../hpp/boundedBuffer.hpp"
#include "../hpp/countingSemaphore.hpp"


using namespace std;

// Constructor
boundedBuffer::boundedBuffer(int size) : maxSize(size)
{

    // Initialize mutex and semaphores
    pthread_mutex_init(&mutex, nullptr);
    empty = new countingSemaphore(maxSize);
    full = new countingSemaphore(0);
}

// Destructor
boundedBuffer::~boundedBuffer()
{

    // Destroy mutex and semaphores
    pthread_mutex_destroy(&mutex);
    delete empty;
    delete full;
}

// Insert an item into the queue
void boundedBuffer::insert(char *s)
{

    // Wait for an empty slot and loock the mutex after that
    empty->down();
    pthread_mutex_lock(&mutex);

    // Insert the item
    q.push(s);

    // Unlock the mutex and signal that a new item is available
    pthread_mutex_unlock(&mutex);
    full->up();
}

char *boundedBuffer::remove()
{
    // Wait for an available item and lock the mutex after that
    full->down();
    pthread_mutex_lock(&mutex);

    // Remove the item
    char *s = q.front();
    q.pop();

    // Unlock the mutex and signal that a slot is now empty
    pthread_mutex_unlock(&mutex);
    empty->up();

    return s;
}

char *boundedBuffer::tryRemove()
{
    // Try to wait for an available item
    if (!full->tryDown())
    {
        return nullptr; 
    }

    // Lock the mutex and remove the item
    pthread_mutex_lock(&mutex);
    char* item = q.front();
    q.pop();

    // Unlock the mutex and signal that a slot is now empty
    pthread_mutex_unlock(&mutex);
    empty->up();

    return item;
}