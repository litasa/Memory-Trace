#ifndef HEAP_H
#define HEAP_H

#include <unordered_map>
#include <map>
#include "core.h"

class Heap {
    public:
    int allocator_id;
    std::string name;
    size_t birth;
    size_t death;
    size_t managed_memory;
    size_t used_memory;
    std::map<size_t,Core> cores_;//pointer to core start, core
    std::unordered_map<size_t,size_t> alloc_to_core; // allocation pointer -> core pointer
    std::vector<Core> recently_dead_cores_;
    void print() const;
    void printAll() const;
    Core* getCore(size_t pointer);
    Core* getCoreForAllocation(size_t pointer);
    void removeCore(Core* core);
};

#endif //HEAP_H