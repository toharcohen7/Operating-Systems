//  

#include "../hpp/producer.hpp"
#include <string>
#include <cstdlib>
#include <ctime>
#include <cstring>

using namespace std;

// Constructor
producer::producer(int id, int pCount, int qSize)
    : id(id), pCount(pCount), qSize(qSize)
{
    // Initiate a queue
    pQueue = new boundedBuffer(qSize);
}

// Destructor
producer::~producer()
{
    // Delete the queue
    delete pQueue;
}

boundedBuffer *producer::getQueue()
{
    return pQueue;
}

// The function that make the strings
void producer::run()
{
    int sportsCounter = 0, weatherCounter = 0, newsCounter = 0, currentCount = 0;

    // Changing the seed to prevent repeated rand sequences with multiple producers
    srand(time(nullptr) + id);

    for (int i = 0; i < pCount; i++)
    {
        // Generate a random type
        int randType = rand() % 3;
        string type;

        switch (randType)
        {
        case 0:
            type = "SPORTS";
            currentCount = sportsCounter++;
            break;
        case 1:
            type = "WEATHER";
            currentCount = weatherCounter++;
            break;
        case 2:
            type = "NEWS";
            currentCount = newsCounter++;
            break;
        }

        // Create the currant message
        string message = "Producer " + to_string(id) + " " + type + " " + to_string(currentCount);

        // Add the message to the queue
        pQueue->insert(strdup(message.c_str()));
    }

    // Send DONE when finished
    pQueue->insert(strdup("DONE"));
}
