
#ifndef SCREEN_MANAGER_HEADER
#define SCREEN_MANAGER_HEADER

#include "boundedBuffer.hpp"

class screenManager
{

private:
    boundedBuffer *queue;

public:
    // Constructor
    screenManager(boundedBuffer *b);

    // Function for managing the screen output
    void manageScreen();
};

#endif /* SCREEN_MANAGER_HEADER */