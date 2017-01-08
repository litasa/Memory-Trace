#ifndef CORE_H
#define CORE_H
#include "allocation.h"
#include "memory_object.h"

#include <unordered_map>

class Core : public MemoryObject {
    public:
    Core(size_t pointer, size_t size, size_t timestamp);

    bool pointerInside(size_t pointer);
    bool allocationInside(size_t pointer, size_t size);
    size_t removeAllocation(size_t pointer, size_t timestamp);
    size_t addAllocation(size_t pointer, size_t size, size_t timestamp);
    Allocation* getAllocation(size_t pointer);

    size_t getManagedSize() { return managed_memory_; }

    private:
    size_t end_;
    size_t managed_memory_;
    size_t used_memory_;

    std::unordered_map<size_t,Allocation> allocations_;//pointer to allocation start, allocation
};

#endif //CORE_H