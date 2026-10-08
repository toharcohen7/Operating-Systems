#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <pthread.h>

#include "../hpp/boundedBuffer.hpp"
#include "../hpp/producer.hpp"
#include "../hpp/dispatcher.hpp"
#include "../hpp/coEditor.hpp"
#include "../hpp/screenManager.hpp"

using namespace std;

// --- Helper Functions for Threads ---
void *runProducer(void *arg)
{
    producer *p = (producer *)arg;
    p->run();
    return nullptr;
}

void *runDispatcher(void *arg)
{
    dispatcher *d = (dispatcher *)arg;
    d->run();
    return nullptr;
}

void *runCoEditor(void *arg)
{
    coEditor *c = (coEditor *)arg;
    c->run();
    return nullptr;
}

void *runScreenManager(void *arg)
{
    screenManager *s = (screenManager *)arg;
    s->manageScreen();
    return nullptr;
}



int main(int argc, char *argv[])
{
    // Validate arguments
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <configuration_file>" << endl;
        return 1;
    }

    // Open configuration file
    ifstream configFile(argv[1]);
    if (!configFile.is_open())
    {
        cerr << "Error opening configuration file." << endl;
        return 1;
    }

    // Read the file and create producers
    vector<producer *> producers;
    int coEditorQueueSize = 0;
    string tempWord;
    
    while (configFile >> tempWord)
    {
        // If it's producer, we read its parameters
        if (tempWord == "PRODUCER")
        {
            int id, products, qSize;
            string trash;

            // Read the id
            configFile >> id;

            // Read the number of products
            configFile >> products;

            // Discard the "queue size =" part and read the size
            configFile >> trash;
            configFile >> trash;
            configFile >> trash;
            configFile >> qSize;

            producers.push_back(new producer(id, products, qSize));
        }
        // If it's "Co-Editor", we read the final size.
        else if (tempWord == "Co-Editor")
        {
            string trash;

            // Discard the "queue size =" part and read the size
            configFile >> trash;
            configFile >> trash;
            configFile >> trash;
            configFile >> coEditorQueueSize;
        }
    }

    configFile.close();

    // Create Queues for Dispatcher -> Co-Editors
    // All 3 queues share the same size defined by 'Co-Editor queue size'
    boundedBuffer *sQueue = new boundedBuffer(coEditorQueueSize);
    boundedBuffer *nQueue = new boundedBuffer(coEditorQueueSize);
    boundedBuffer *wQueue = new boundedBuffer(coEditorQueueSize);

    // Create Queue for Co-Editors -> ScreenManager
    // Usually implicitly same size, or large enough
    boundedBuffer *screenQueue = new boundedBuffer(coEditorQueueSize);

    // Create System Objects
    dispatcher *disp = new dispatcher(producers, sQueue, nQueue, wQueue);

    coEditor *sportsEditor = new coEditor(sQueue, screenQueue);
    coEditor *newsEditor = new coEditor(nQueue, screenQueue);
    coEditor *weatherEditor = new coEditor(wQueue, screenQueue);

    screenManager *screenMgr = new screenManager(screenQueue);

    // Create Threads
    vector<pthread_t> producerThreads(producers.size());
    pthread_t dispatcherThread;
    pthread_t sThread, nThread, wThread;
    pthread_t screenThread;

    // Start Producers
    for (size_t i = 0; i < producers.size(); ++i)
    {
        pthread_create(&producerThreads[i], nullptr, runProducer, producers[i]);
    }

    // Start Dispatcher
    pthread_create(&dispatcherThread, nullptr, runDispatcher, disp);

    // Start Co-Editors
    pthread_create(&sThread, nullptr, runCoEditor, sportsEditor);
    pthread_create(&nThread, nullptr, runCoEditor, newsEditor);
    pthread_create(&wThread, nullptr, runCoEditor, weatherEditor);

    // Start Screen Manager
    pthread_create(&screenThread, nullptr, runScreenManager, screenMgr);

    // Wait for Threads to Finish (Join)
    for (size_t i = 0; i < producers.size(); ++i)
    {
        pthread_join(producerThreads[i], nullptr);
    }

    pthread_join(dispatcherThread, nullptr);

    pthread_join(sThread, nullptr);
    pthread_join(nThread, nullptr);
    pthread_join(wThread, nullptr);

    pthread_join(screenThread, nullptr);

    // Cleanup Memory
    // Delete producers
    for (auto p : producers)
    {
        delete p;
    }

    // Delete system objects
    delete disp;
    delete sportsEditor;
    delete newsEditor;
    delete weatherEditor;
    delete screenMgr;

    // Delete queues
    delete sQueue;
    delete nQueue;
    delete wQueue;
    delete screenQueue;

    return 0;
}