#ifndef MEMORY_STATE_H
#define MEMORY_STATE_H

#include <string>
#include <unordered_map>
#include <map>
#include <iostream>

class Allocation {
    public:
    int allocator_id;
    size_t pointer;
    size_t size;
    size_t time_stamp;
};

class Core {
    public:
    int allocator_id;
    size_t pointer;
    size_t size;
    size_t time_stamp;
    std::unordered_map<size_t,Allocation> allocations_;//pointer, allocation
    void print() const;
};

class Allocator {
    public:
    int allocator_id;
    std::string name;
    size_t time_stamp;
    std::map<size_t,Core> cores_;//pointer, core
    std::unordered_map<size_t,size_t> alloc_to_core; // allocation pointer -> core pointer
    void print() const;
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

    void print(size_t timestamp) const;

    int times_called;
private:
    std::unordered_map<int,Allocator> allocators_;
};

#endif