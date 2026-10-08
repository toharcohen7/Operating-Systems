#ifndef CO_EDITOR_HEADER
#define CO_EDITOR_HEADER

#include "boundedBuffer.hpp"

class coEditor
{
private:
    boundedBuffer* inQueue;
    boundedBuffer* outQueue;
public:

    // Constructor
    coEditor(boundedBuffer* input, boundedBuffer* output);

    void run();
};

#endif /* CO_EDITOR_HEADER */