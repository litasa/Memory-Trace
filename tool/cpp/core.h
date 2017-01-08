#ifndef CORE_H
#define CORE_H
#include "allocation.h"

#include <unordered_map>

class Core {
    public:
    Core(size_t pointer, size_t size, size_t timestamp);

    void print() const;
    void printAll() const;
    bool pointerInside(size_t pointer);
    bool allocationInside(size_t pointer, size_t size);
    size_t removeAllocation(size_t pointer, size_t timestamp);
    size_t addAllocation(size_t pointer, size_t size, size_t timestamp);
    Allocation* getAllocation(size_t pointer);

    size_t getManagedSize() { return managed_memory_; }
    size_t getPointer() { return start_; }

    size_t getLastUpdate() const { return last_update_; }

    void setLastUpdate(size_t update) { last_update_ = update;}

    private:
    size_t start_;
    size_t end_;
    size_t birth_;
    size_t death_;
    size_t managed_memory_;
    size_t used_memory_;

    size_t last_update_;

    std::unordered_map<size_t,Allocation> allocations_;//pointer to allocation start, allocation
};

#endif //CORE_H