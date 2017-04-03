#ifndef HEAP_H
#define HEAP_H

#include <unordered_map>
#include <map>
#include "core.h"
#include "memory_object.h"

struct heap_usage {
        heap_usage(size_t u = 0, size_t m = 0) : used_memory(u), managed_memory(m) {};
        size_t used_memory;
        size_t managed_memory;
    };

class Heap : public MemoryObject {
public:
    Heap(size_t timestamp, size_t id, std::string type, std::string name);
    ~Heap();
    
    std::map<size_t,Core> cores_;//pointer to core start, core
    std::map<size_t,Allocation> allocations_;
    std::unordered_map<size_t,size_t> alloc_to_core; // allocation pointer -> core pointer
    std::map<size_t,heap_usage> simple_allocation_events_; //timestamp, used_size;
    Core* getCore(size_t pointer);
    Core* getCoreForAllocation(size_t pointer);

    bool removeCore(size_t timestamp, size_t pointer);
    bool shrinkCore(size_t timestamp, size_t pointer, size_t size);
    bool addCore(size_t timestamp, size_t pointer, size_t managed_size);
    bool growCore(size_t timestamp, size_t pointer, size_t size);

    bool removeAllocation(size_t timestamp, size_t pointer, bool core_exist);
    bool addAllocation(size_t timestamp, size_t pointer, size_t size, bool core_exist);

    bool setBackingAllocator(size_t alloc);
    std::string getName() const { return name_; }
    std::string getType() const { return type_; }
    void printBacking();

    void printContent() const;

    std::vector<size_t> backing_allocator_ids;
    std::map<size_t,Core> recently_dead_;
    bool dead = false;
    const size_t id_;
    const std::string type_;
    const std::string name_;
private:
    
};

#endif //HEAP_H