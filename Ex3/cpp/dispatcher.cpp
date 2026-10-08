
#include "../hpp/dispatcher.hpp"
#include <string>
#include <cstring>

using namespace std;

// Constructor
dispatcher::dispatcher(vector<producer *> &producers, boundedBuffer *sQueue, boundedBuffer *nQueue, boundedBuffer *wQueue)
    : producers(producers), sQueue(sQueue), wQueue(wQueue), nQueue(nQueue)
{
    pNum = producers.size();
}

// Destructor
dispatcher::~dispatcher()
{
}

// The function that run the dispatcher
void dispatcher::run()
{
    int finishedProducers = 0;

    // Loop until all producers have sent "DONE"
    while (finishedProducers < pNum)
    {

        // Round robin through each producer
        for (int i = 0; i < pNum; ++i)
        {
            boundedBuffer *currentQueue = producers[i]->getQueue();

            // Try to remove without blocking
            char *msg = currentQueue->tryRemove();

            if (msg != nullptr)
            {
                string msgStr = string(msg);

                // Check for "DONE" message
                if (strcmp(msg, "DONE") == 0)
                {
                    finishedProducers++;
                    free(msg);
                    continue;
                }
                else
                {
                    // Determine the type and insert into the appropriate queue
                    if (msgStr.find("SPORTS") != string::npos)
                    {
                        sQueue->insert(msg);
                    }
                    else if (msgStr.find("NEWS") != string::npos)
                    {
                        nQueue->insert(msg);
                    }
                    else if (msgStr.find("WEATHER") != string::npos)
                    {
                        wQueue->insert(msg);
                    }
                    else
                    {
                        // Free memory to avoid leak for unknown type
                        free(msg);
                    }
                }
            }
        }
    }

    // After all producers are done, send "DONE" to each queue
    sQueue->insert(strdup("DONE"));
    nQueue->insert(strdup("DONE"));
    wQueue->insert(strdup("DONE"));
}