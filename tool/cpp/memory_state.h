#ifndef MEMORY_STATE_H
#define MEMORY_STATE_H

#include <string>
#include <unordered_map>
#include <map>
#include <iostream>
#include "core.h"
#include "allocation.h"
#include "heap.h"
#include "event.h"

class MemoryState {
public:
    MemoryState();
    ~MemoryState();

    void setInits(size_t stream_magic, std::string platform, size_t frequency);

    void addEvent(Event::Event* event);

    bool addHeap(size_t timestamp, const size_t id, const std::string& type, const std::string& name);
    bool addCore(size_t timestamp, const size_t id, const size_t pointer, const size_t size);
    bool growCore(size_t timestamp, const size_t id, const size_t pointer, const size_t size);
    bool addAllocation(size_t timestamp, const size_t id, const size_t pointer, const size_t size);

    bool removeHeap(size_t timestamp, const size_t id);
    bool removeCore(size_t timestamp, const size_t id, const size_t pointer, const size_t size);
    bool shrinkCore(size_t timestamp, const size_t id, const size_t pointer, const size_t size);    
    bool removeAllocation(size_t timestamp, const size_t id, const size_t pointer);
    bool removeAllAllocations(size_t timestamp, const size_t id);

    void print(size_t timestamp, size_t id = -1) const;
    void printAll() const;

    Heap* getHeap(const size_t id);

    void printStats();

    double frequency_;
    std::vector<Heap*> getHeaps();
    std::vector<Heap> dead_heaps;
private:
    bool debug = false;
    std::vector<Event::Event*> eventList;
    std::unordered_map<size_t, Heap> heaps_;
    std::string platform_;
    size_t stream_magic_;
    size_t last_update_ = 0;

    int num_heaps_added = 0;
    int num_heaps_removed = 0;

    int num_cores_added = 0;
    int num_cores_removed = 0;

    int num_allocations_added = 0;
    int num_allocations_removed = 0;
};

#endif