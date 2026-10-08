

#include "../hpp/coEditor.hpp"
#include <unistd.h>
#include <cstring>

// Constructor
coEditor::coEditor(boundedBuffer *input, boundedBuffer *output)
    : inQueue(input), outQueue(output)
{
}

void coEditor::run()
{
    while (true)
    {
        char *msg = inQueue->remove();

        // Check for "DONE" signal
        if (strcmp(msg, "DONE") == 0) {
            outQueue->insert(msg);
            break;
        }

        // Simulate editing by delay of 0.1 seconds
        usleep(100000);

        // Insert the edited message
        outQueue->insert(msg);
    }
}