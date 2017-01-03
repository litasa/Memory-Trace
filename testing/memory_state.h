#ifndef MEMORY_STATE_H
#define MEMORY_STATE_H

#include <string>
#include <unordered_map>
#include <map>
#include <iostream>
#include "ringbuffer.h"

class Allocation {
    public:
    int allocator_id;
    size_t pointer;
    size_t size;
    size_t birth;
    size_t death;
};

class Core {
    public:
    int allocator_id;
    size_t pointer;
    size_t birth;
    size_t death;
    size_t managed_memory;
    size_t used_memory;
    std::unordered_map<size_t,Allocation> allocations_;//pointer to allocation start, allocation
    std::vector<Allocation> recently_dead_allocations_;
    void print() const;
    bool pointerInside(size_t pointer);
    bool allocationInside(size_t pointer, size_t size);
    void removeAllocation(Allocation* allocation);
    Allocation* getAllocation(size_t pointer);
};

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
    Core* getCore(size_t pointer);
    Core* getCoreForAllocation(size_t pointer);
    void removeCore(Core* core);
};


class MemoryState {
public:
    MemoryState();
    ~MemoryState();

    void addHeap(const int id, const std::string& name, size_t timestamp);
    void addCore(const int id, const size_t pointer, const size_t size, size_t timestamp);
    void addAllocation(const int id, const size_t pointer, const size_t size, size_t timestamp);

    void removeHeap(const int id, size_t timestamp);
    void removeCore(const int id, const size_t pointer, const size_t size, size_t timestamp);
    void removeAllocation(const int id, const size_t pointer, size_t timestamp);

    void print(size_t timestamp, int id = -1) const;

    Heap* getHeap(const int id);
private:

    std::unordered_map<int, Heap> heaps_;
    std::vector<Heap> recently_dead_heaps_;
};

#endif