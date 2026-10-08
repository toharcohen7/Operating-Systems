
#include "../hpp/screenManager.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;

// Constructor
screenManager::screenManager(boundedBuffer *q) : queue(q) {}

// Function for managing the screen output
void screenManager::manageScreen()
{
    int doneCount = 0;

    while (doneCount < 3)
    {
        // Remove message from the queue
        char *msg = queue->remove();

        // Check for DONE message
        if (strcmp(msg, "DONE") == 0)
        {
            doneCount++;
            free(msg);
        }
        else
        {
            // Print the message and free memory
            cout << msg << endl;
            free(msg);
        }
    }

    // Print final DONE message
    cout << "DONE" << endl;
}