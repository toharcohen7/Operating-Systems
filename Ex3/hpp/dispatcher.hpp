
#ifndef DISPATCHER_HEADER
#define DISPATCHER_HEADER

#include "producer.hpp"
#include "boundedBuffer.hpp"

using namespace std;

class dispatcher
{
private:

    // List of producers and producers number
    vector<producer *> producers;
    int pNum;

    // Queue for each type
    boundedBuffer *sQueue;
    boundedBuffer *wQueue;
    boundedBuffer *nQueue;

public:
    // Constructor
    dispatcher(vector<producer*>& producers, boundedBuffer* sQueue, boundedBuffer* nQueue, boundedBuffer* wQueue);

    // Destructor
    ~dispatcher();

    // The function that run the dispatcher
    void run();
};

#endif /* DISPATCHER_HEADER */