//  

#ifndef PRODUCER_HEADER
#define PRODUCER_HEADER

#include "boundedBuffer.hpp"

class producer
{

private:
    int id;
    int pCount;
    int qSize;
    boundedBuffer* pQueue;

public:
    // Constructor
    producer(int id, int pCount, int qSize);

    // distructor
    ~producer();

    // The function that run the producer 
    void run();

    // Getter for the producer queue
    boundedBuffer* getQueue();

};

#endif /* PRODUCER_HEADER */
