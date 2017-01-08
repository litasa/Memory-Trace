#include "heap.h"

#include <iostream>

Heap::Heap(int id,
    std::string name,
    size_t base,
    size_t timestamp)
    : 
    MemoryObject(base, timestamp, 0, 0), 
    id_(id),
    name_(name)
    { }

Core* Heap::getCore(size_t pointer) {
    auto found = cores_.find(pointer);
    if(found == cores_.end()) {
        std::cout << "No core found" << std::endl;
        return nullptr;
    }
    return &(found->second);
}

Core* Heap::getCoreForAllocation(size_t pointer) {
    auto found = alloc_to_core.find(pointer);
    if(found == alloc_to_core.end()) {
        std::cout << "No core mapper Found" << std::endl;
        return nullptr;
    }
    return getCore(found->second);
}

size_t Heap::removeCore(size_t pointer, size_t timestamp) {
    Core core = cores_.at(pointer);
    size_t managed = core.getManagedSize();
    size_t items_removed = cores_.erase(core.getPointer());
    if(items_removed == 1) {
        managed_memory_ -= managed;
        return managed;
    }
    return 0;
}

size_t Heap::addCore(size_t pointer, size_t timestamp, size_t managed_size) {
    Core c(pointer,timestamp,managed_size);
    auto emp = cores_.insert(std::make_pair(pointer, c));
    if(!emp.second) {
        //std::cout << "Adding Core failed: " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " timestamp: " << timestamp  << "\n";
        return 0;
    }
    managed_memory_ += managed_size;
    return managed_size;
}

size_t Heap::addAllocation(size_t pointer, size_t size, size_t timestamp) {
    for(auto it = cores_.begin(); it != cores_.end(); ++it) {
        if(it->second.addAllocation(pointer, size, timestamp)) {
            alloc_to_core.emplace(pointer,it->second.getPointer());
            used_memory_ += size;
            return size;
        }
    }
    std::cout << "Did not find core for: " << std::hex << pointer << std::dec << " in heap: " << id_;
    return 0;
}

size_t Heap::removeAllocation(size_t pointer, size_t timestamp) {
    Core* core = getCoreForAllocation(pointer);
    if(core == nullptr) {
        return 0;
    }
    size_t removed_memory = core->removeAllocation(pointer,timestamp);
    used_memory_ -= removed_memory;
    return removed_memory;
}