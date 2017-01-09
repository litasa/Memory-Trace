#ifndef MEMORY_STATE_H
#define MEMORY_STATE_H

#include <string>
#include <unordered_map>
#include <map>
#include <iostream>
#include "core.h"
#include "allocation.h"
#include "heap.h"

class MemoryState {
public:
    MemoryState();
    ~MemoryState();

    bool addHeap(const int id, const std::string& name, size_t timestamp);
    bool addCore(const int id, const size_t pointer, const size_t size, size_t timestamp);
    bool addAllocation(const int id, const size_t pointer, const size_t size, size_t timestamp);

    bool removeHeap(const int id, size_t timestamp);
    bool removeCore(const int id, const size_t pointer, const size_t size, size_t timestamp);
    bool removeAllocation(const int id, const size_t pointer, size_t timestamp);

    void print(size_t timestamp, int id = -1) const;
    void printAll() const;

    Heap* getHeap(const int id);

    void printStats();

    std::vector<Heap*> getHeaps();
private:
    std::unordered_map<int, Heap> heaps_;
    size_t last_update_ = 0;
    //std::vector<Heap> recently_dead_heaps_;

    int num_heaps_added = 0;
    int num_heaps_removed = 0;

    int num_cores_added = 0;
    int num_cores_removed = 0;

    int num_allocations_added = 0;
    int num_allocations_removed = 0;
};

#endif