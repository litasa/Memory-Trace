#ifndef CORE_H
#define CORE_H
#include "allocation.h"

#include <unordered_map>
#include <stdlib.h> //size_t

class Core {
    public:
    int allocator_id;
    size_t pointer;
    size_t birth;
    size_t death;
    size_t managed_memory;
    size_t used_memory;
    std::unordered_map<size_t,Allocation> allocations_;//pointer to allocation start, allocation
    void print() const;
    void printAll() const;
    bool pointerInside(size_t pointer);
    bool allocationInside(size_t pointer, size_t size);
    void removeAllocation(Allocation* allocation);
    Allocation* getAllocation(size_t pointer);
};

#endif //CORE_H
