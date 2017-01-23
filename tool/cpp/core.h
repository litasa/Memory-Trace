#ifndef CORE_H
#define CORE_H
#include "allocation.h"
#include "memory_object.h"

#include <unordered_map>

class Core : public MemoryObject {
    public:
    Core(size_t pointer, size_t size, size_t timestamp);

    //~Core();

    bool pointerInside(size_t pointer);
    bool allocationInside(size_t pointer, size_t size);
    bool removeAllocation(size_t pointer, size_t timestamp);
    bool addAllocation(size_t pointer, size_t size, size_t timestamp);
    Allocation* getAllocation(size_t pointer);

    size_t getManagedSize() { return managed_memory_; }
    size_t getNumberOfAllocations() { return allocations_.size(); }

    void printContent() const;

size_t end_;
    size_t managed_memory_;
    size_t used_memory_;
    std::unordered_map<size_t,Allocation> allocations_;//pointer to allocation start, allocation
    private:
    

    
};

#endif //CORE_H