#include "core.h"
#include <iostream>

Core::Core(size_t pointer, size_t size, size_t timestamp) 
        : start_(pointer), managed_memory_(size), birth_(timestamp), last_update_(timestamp)
{
    end_ = pointer + managed_memory_;
}

void Core::print() const {
    for ( auto it = allocations_.cbegin(); it != allocations_.cend(); ++it ) {
        std::cout << "\t\t\t" << std::showbase << std::hex << it->first  << std::dec << " : " << it->second.getSize() << "\n";  // cannot modify *it
    }
}

void Core::printAll() const {
    if(!allocations_.empty())
    {
        std::cout << "\t\tAlive Content: \n";
        print();
    }
}

bool Core::pointerInside(size_t pointer) {
    return (start_ <= pointer) && (end_ >= pointer);
}

bool Core::allocationInside(size_t pointer, size_t size) {
    return (start_ <= pointer) && (end_ >= pointer + size);
}

Allocation* Core::getAllocation(size_t pointer) {
    auto found = allocations_.find(pointer);
    if(found == allocations_.end()) {
        std::cout << "No allocation Found" << std::endl;
        return nullptr;
    }
    return &(found->second);
}

size_t Core::removeAllocation(size_t pointer, size_t timestamp) {
    size_t bytes = allocations_.at(pointer).getSize();
    size_t num_removed = allocations_.erase(pointer);
    if(num_removed == 1) {
        used_memory_ -= bytes;
        setLastUpdate(timestamp);
        return bytes;
    }
    return 0;
}

size_t Core::addAllocation(size_t pointer, size_t size, size_t timestamp) {
    if(allocationInside(pointer,size)) {
        Allocation a(pointer, size, timestamp);
        auto emp = allocations_.insert(std::make_pair(pointer,a));
        if(!emp.second) {
            //std::cout << "Adding Allocation failed (already exists): " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " created: " << timestamp << "\n";
            //std::cout << "had values: " << emp.first->second.allocator_id << " pointer: " << std::hex << emp.first->second.pointer << std::dec << " size: " << emp.first->second.size << " created: " << emp.first->second.birth << "\n";
            return 0;
        }
        used_memory_ += size;
        setLastUpdate(timestamp);
        return size;
    }
    return 0;
}