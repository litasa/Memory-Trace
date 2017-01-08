#include "heap.h"

#include <iostream>

Heap::Heap(int id,
    std::string name,
    size_t birth)
    : 
    id_(id),
    name_(name),
    birth_(birth),
    death_(-1),
    last_update_(birth),
    managed_memory_(0),
    used_memory_(0)
    {

    }

void Heap::print() const {
    for ( auto it = cores_.cbegin(); it != cores_.cend(); ++it ) {
        //std::cout << "\t\t" << std::showbase << std::hex << it->first << std::dec << " : " << it->second.used_memory <<  " // "  << it->second.managed_memory << "\n";  // cannot modify *it
         it->second.print();
     }
}

void Heap::printAll() const {
    if(!cores_.empty())
    {
        std::cout << "\tAlive Content: \n";
        print();
    }
    if(!alloc_to_core.empty()) {
        std::cout << "\tSome mappings still exist\n";
    }
}

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