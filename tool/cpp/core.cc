#include "core.h"
#include <iostream>

void Core::print() const {
    for ( auto it = allocations_.cbegin(); it != allocations_.cend(); ++it ) {
        std::cout << "\t\t\t" << std::showbase << std::hex << it->first  << std::dec << " : " << it->second.size << "\n";  // cannot modify *it
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
    return (this->pointer <= pointer) && (this->pointer + this->managed_memory >= pointer);
}

bool Core::allocationInside(size_t pointer, size_t size) {
    return (this->pointer <= pointer) && (this->pointer + this->managed_memory >= pointer + size);
}

Allocation* Core::getAllocation(size_t pointer) {
    auto found = allocations_.find(pointer);
    if(found == allocations_.end()) {
        return nullptr;
    }
    return &(found->second);
}

void Core::removeAllocation(Allocation* allocation) {
    auto found = allocations_.find(allocation->pointer);
    if(found == allocations_.end()) {
        std::cout << "could not find allocation for removal\n";
    }
    allocations_.erase(allocation->pointer);
}