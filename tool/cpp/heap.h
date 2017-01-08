#ifndef HEAP_H
#define HEAP_H

#include <unordered_map>
#include <map>
#include "core.h"

class Heap {
public:
    Heap(int id, std::string name, size_t birth);

    std::map<size_t,Core> cores_;//pointer to core start, core
    std::unordered_map<size_t,size_t> alloc_to_core; // allocation pointer -> core pointer
    //std::vector<Core> recently_dead_cores_;
    void print() const;
    void printAll() const;
    Core* getCore(size_t pointer);
    Core* getCoreForAllocation(size_t pointer);
    size_t removeCore(size_t pointer, size_t timestamp);
    size_t addCore(size_t pointer, size_t timestamp, size_t managed_size);
    size_t addAllocation(size_t pointer, size_t size, size_t timestamp);

    size_t removeAllocation(size_t pointer, size_t timestamp);

    std::string getName() const { return name_; }
    size_t getManagedMemory() const { return managed_memory_; }
    size_t getUsedMemory() const { return used_memory_; }
    size_t getLastUpdate() const { return last_update_; }

    void setLastUpdate(size_t update) { last_update_ = update;}

private:
    const int id_;
    const std::string name_;
    const size_t birth_;
    size_t death_;
    size_t last_update_;
    size_t managed_memory_;
    size_t used_memory_;
};

#endif //HEAP_H