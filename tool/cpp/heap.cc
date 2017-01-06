#include "heap.h"

#include <iostream>

void Heap::print() const {
    for ( auto it = cores_.cbegin(); it != cores_.cend(); ++it ) {
        std::cout << "\t\t" << std::showbase << std::hex << it->first << std::dec << " : " << it->second.used_memory <<  " // "  << it->second.managed_memory << "\n";  // cannot modify *it
         it->second.print();
     }
}

void Heap::printAll() const {
    if(!cores_.empty())
    {
        std::cout << "\tAlive Content: \n";
        print();
    }
    if(!recently_dead_cores_.empty()) {
        std::cout << "\tDead Content: \n";
        for ( auto it = recently_dead_cores_.cbegin(); it != recently_dead_cores_.cend(); ++it ) {
            std::cout << "\t\t" << std::showbase << std::hex << it->pointer  << std::dec << " : " << it->managed_memory << " | B: " << it->birth << ", D: " << it->death <<  "\n";  // cannot modify *it
            it->printAll();
        }
    }
    if(!alloc_to_core.empty()) {
        std::cout << "\tSome mappings still exist\n";
    }
}

Core* Heap::getCore(size_t pointer) {
    auto found = cores_.find(pointer);
    if(found == cores_.end()) {
        for(auto it = recently_dead_cores_.begin(); it != recently_dead_cores_.end(); ++it) {
            if(it->pointer == pointer) {
                std::cout << "getting dead core for pointer: " << std::hex << pointer << std::dec << "\n";
                return &(*it);
            }
        }
        return nullptr;
    }
    return &(found->second);
}

Core* Heap::getCoreForAllocation(size_t pointer) {
    auto found = alloc_to_core.find(pointer);
    if(found == alloc_to_core.end()) {
        for(auto it = recently_dead_cores_.begin(); it != recently_dead_cores_.end(); ++it) {
            if(it->pointer == pointer) {
                std::cout << "getting dead core for pointer: " << std::hex << pointer << std::dec << "\n";
                return &(*it);
            }
        }
        return nullptr;
    }
    return getCore(found->second);
}

void Heap::removeCore(Core* core) {
    if(!core->allocations_.empty()) {
        recently_dead_cores_.push_back(*core);
    }
    cores_.erase(core->pointer);
}