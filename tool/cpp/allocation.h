#ifndef ALLOCATION_H
#define ALLOCATION_H
#include "memory_object.h"

class Allocation : public MemoryObject{
public:
    Allocation::Allocation(size_t pointer, size_t size, size_t timestamp)
    : MemoryObject(pointer, timestamp, size, 0)
    {}
};
#endif //ALLOCATION_H