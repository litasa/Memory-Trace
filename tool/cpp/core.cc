#include "core.h"
#include <iostream>

Core::Core(size_t timestamp, size_t pointer, size_t size) 
        : MemoryObject(timestamp, pointer, 0, size)
{
    end_ = pointer + size;
}

void Core::printContent() const {
    //std::cout << "\tcore: " << std::hex << getPointer() << std::dec << " managed: " <<  getManagedMemory() << " size: " <<  getUsedMemory() << " at timestmap: " << getBirth() << "\n";
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
        //std::cout << "No allocation Found" << std::endl;
        return nullptr;
    }
    return &(found->second);
}

size_t Core::removeAllocation(size_t timestamp, size_t pointer, bool debug) {
    auto it = allocations_.find(pointer);
    if(it == allocations_.end()) {
        if(debug) {std::cout << "Remove allocation failed in core" << std::endl;    }
        return 0;
    }
    size_t bytes = it->second.getUsedMemory();
    allocations_.erase(it);
    used_memory_ -= bytes;
    setLastUpdate(timestamp);
    return bytes;
}

bool Core::addAllocation(size_t timestamp, size_t pointer, size_t size) {
    //std::cout << "trying to add allocation: " << std::hex << pointer << std::dec << " size: " << size << " at: " << timestamp << " ";
    if(allocationInside(pointer,size)) {
        Allocation a(pointer, size, timestamp);
        auto emp = allocations_.insert(std::make_pair(pointer,a));
        if(!emp.second) {
            //std::cout << "Adding Allocation failed (already exists in core): " << std::hex << this->pointer_ << std::dec << " size: " << this->managed_memory_ << " created: " << this->birth_ << "\n";
            return false;
        }
        used_memory_ += size;
        setLastUpdate(timestamp);
        return true;
    }
    // std::cout << "Adding Allocation failed: " << std::hex << pointer << std::dec << " size: " << size << " at timestmap: " << timestamp << "\n";
    // std::cout << "\t(is not inside core): " << std::hex << this->pointer_ << " end: " << end_ << std::dec << " size: " << this->managed_memory_ << " created: " << this->birth_ << "\n";    
    // std::cout << "\tAllocation: " << std::hex << pointer << std::dec << ", size: " << size << ", timestamp: " << timestamp << "\n";
    // std::cout << "\tCore: " << std::hex << pointer_ << ", end: " << end_ << std::dec << ", bytes left: " << managed_memory_ - used_memory_ <<"\n";
    return false;
}