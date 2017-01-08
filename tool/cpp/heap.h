#ifndef HEAP_H
#define HEAP_H

#include <unordered_map>
#include <map>
#include "core.h"
#include "memory_object.h"

class Heap : public MemoryObject {
public:
    Heap(int id, std::string name, size_t base, size_t birth);

    std::map<size_t,Core> cores_;//pointer to core start, core
    std::unordered_map<size_t,size_t> alloc_to_core; // allocation pointer -> core pointer
    Core* getCore(size_t pointer);
    Core* getCoreForAllocation(size_t pointer);
    size_t removeCore(size_t pointer, size_t timestamp);
    size_t addCore(size_t pointer, size_t timestamp, size_t managed_size);
    size_t addAllocation(size_t pointer, size_t size, size_t timestamp);

    size_t removeAllocation(size_t pointer, size_t timestamp);

    std::string getName() const { return name_; }

private:
    const int id_;
    const std::string name_;
};

#endif //HEAP_H