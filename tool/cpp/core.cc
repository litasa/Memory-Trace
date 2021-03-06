#include "core.h"
#include <iostream>

Core::Core(size_t timestamp, size_t pointer, size_t size) 
        : MemoryObject(timestamp, pointer, 0, size)
{
    end_ = pointer + size;
}

void Core::printContent() const {
    std::cout << "\tcore: " << std::hex << getPointer() << " end: " << end_ << std::dec << " managed: " <<  getManagedMemory() << " size: " <<  getUsedMemory() << " at timestmap: " << getBirth() << "\n";
    for(auto it = allocations_.begin(); it != allocations_.end(); ++it) {
        it->second.printContent();
    }
}

bool Core::pointerInside(size_t pointer) {
    return (getPointer() <= pointer) && (end_ >= pointer);
}

bool Core::allocationInside(size_t pointer, size_t size) {
    return (getPointer() <= pointer) && (end_ >= (pointer + size));
}

Allocation* Core::getAllocation(size_t pointer) {
    auto found = allocations_.find(pointer);
    if(found == allocations_.end()) {
        return nullptr;
    }
    return &(found->second);
}

size_t Core::removeAllocation(size_t timestamp, size_t pointer) {
    auto it = allocations_.find(pointer);
    if(it == allocations_.end()) {
        return 0;
    }
    size_t bytes = it->second.getUsedMemory();
    allocations_.erase(it);
    used_memory_ -= bytes;
    setLastUpdate(timestamp);
    return bytes;
}

bool Core::addAllocation(size_t timestamp, size_t pointer, size_t size, bool force) {
    if(allocationInside(pointer,size)) {
        Allocation a(pointer, size, timestamp);
        auto emp = allocations_.insert(std::make_pair(pointer,a));
        if(!emp.second) {
            return false;
        }
        used_memory_ += size;
        setLastUpdate(timestamp);
        return true;
    }
    if(force) {
        Allocation a(pointer, size, timestamp);
        auto emp = allocations_.insert(std::make_pair(pointer,a));
        if(!emp.second) {
            return false;
        }
        used_memory_ += size;
        setLastUpdate(timestamp);
        return true;
    }
    return false;
}

bool Core::removeAllAllocations() {
    allocations_.clear();
    return true;
}

void Core::shrink(size_t size) {
    managed_memory_ -= size;
    end_ -= size;
    if(managed_memory_ < used_memory_) {
        std::cout << "shrunk the managed memory below used memory" << std::endl;
    }
}

void Core::grow(size_t size) {
    managed_memory_ += size;
    end_ += size;
}