#ifndef HEAP_H
#define HEAP_H

#include <unordered_map>
#include <map>
#include "core.h"
#include "memory_object.h"

class Heap : public MemoryObject {
public:
    Heap(size_t timestamp, int id, std::string type, std::string name);
    ~Heap();

    std::map<size_t,Core*> cores_;//pointer to core start, core
    std::unordered_map<size_t,size_t> alloc_to_core; // allocation pointer -> core pointer
    std::vector<std::pair<size_t, size_t>> simple_allocation_events_; //timestamp, used_size;
    Core* getCore(size_t pointer);
    Core* getCoreForAllocation(size_t pointer);

    bool removeCore(size_t timestamp, size_t pointer);
    bool addCore(size_t timestamp, size_t pointer, size_t managed_size);

    bool removeAllocation(size_t timestamp, size_t pointer);
    bool addAllocation(size_t timestamp, size_t pointer, size_t size);

    bool setBackingAllocator(unsigned int alloc);
    std::string getName() const { return name_; }
    void printBacking();

    void printContent() const;

    std::vector<int> backing_allocator_ids;
    std::map<size_t,Core*> recently_dead_;
    bool dead = false;
    const int id_;
    const std::string type_;
    const std::string name_;
    bool own_core_;
private:
    
};

#endif //HEAP_H