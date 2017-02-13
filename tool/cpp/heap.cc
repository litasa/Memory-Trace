#include "heap.h"

#include <iostream>

Heap::Heap(size_t timestamp, size_t id, std::string type, std::string name)
    : MemoryObject(timestamp, 0, 0, 0), id_(id), type_(type), name_(name)
{
        
}

Heap::~Heap() {
    //maps manages themselves
}

void Heap::printContent() const {
    std::cout << "printing content for heap " << id_ << " of type: "<< type_.c_str() << "\n";
    for(auto it = cores_.begin(); it != cores_.end(); ++it) {
        it->second.printContent();
    }
}

Core* Heap::getCore(size_t pointer) {
    auto found = cores_.find(pointer);
    if(found == cores_.end()) {
        return nullptr;
    }
    return (&found->second);
}

Core* Heap::getCoreForAllocation(size_t pointer) {
    auto found = alloc_to_core.find(pointer);
    if(found == alloc_to_core.end()) {
        return nullptr;
    }
    return getCore(found->second);
}

bool Heap::removeCore(size_t timestamp, size_t pointer) {
    auto it = cores_.find(pointer);
    if(it == cores_.end()) {
        std::cout << "remove Core failed in heap " << id_ << std::endl;
        return false;
    }
    size_t managed = it->second.getManagedSize();
    managed_memory_ -= managed;
    cores_.erase(it);
    return true;
}

bool Heap::addCore(size_t timestamp, size_t pointer, size_t managed_size) {
    Core c(timestamp, pointer, managed_size);
    auto emp = cores_.insert(std::make_pair(pointer, c));
    if(!emp.second) {
        std::cout << "tryint to add already existing core: " << std::hex << pointer << std::dec << std::endl;
        return false;
    }
    managed_memory_ += managed_size;
    return true;
}

bool Heap::growCore(size_t timestamp, size_t pointer, size_t size) {
    Core* core = getCore(pointer);
    if(core != nullptr) {
        core->grow(size);
        return true;    
    }
    return false;
}

bool Heap::shrinkCore(size_t timestamp, size_t pointer, size_t size) {
    Core* core = getCore(pointer);
    if(core != nullptr) {
        core->shrink(size);
        return true;    
    }
    return false;
}

bool Heap::addAllocation(size_t timestamp, size_t pointer, size_t size) {
        for(auto it = cores_.begin(); it != cores_.end(); ++it) {
            if(it->second.addAllocation(timestamp, pointer, size)) {
                alloc_to_core.emplace(pointer,it->second.getPointer());
                used_memory_ += size;
                simple_allocation_events_[timestamp] = heap_usage(used_memory_, managed_memory_);
                return true;
            }
        }
        //does the allocation span over two cores?
        for(auto it = cores_.begin(); it != cores_.end(); ++it) {
            auto next = std::next(it);
            if(next != cores_.end()) {
                if(it->second.pointerInside(pointer) && next->second.pointerInside(pointer + size)) {
                    it->second.addAllocation(timestamp,pointer,size, true);
                    alloc_to_core.emplace(pointer,it->second.getPointer());
                    used_memory_ += size;
                    simple_allocation_events_[timestamp] = heap_usage(used_memory_, managed_memory_);
                    return true;                 
                }
            }
        }
        std::cout << "did not find a core for allocation in heap: " << id_ << " pointer: " << std::hex << pointer << std::dec << " size: " << size << " at timestamp " << timestamp << "\n";
    return false;
}

bool Heap::removeAllocation(size_t timestamp, size_t pointer) {
    Core* core = getCoreForAllocation(pointer);
    if(core == nullptr) {
        std::cout << "core was null for: " << std::hex << pointer << std::dec << "at " << timestamp << std::endl;
        printContent();
        return false;
    }
    size_t removed_memory = core->removeAllocation(timestamp, pointer);
    used_memory_ -= removed_memory;
    simple_allocation_events_[timestamp] = heap_usage(used_memory_, managed_memory_);
    return true;
}

bool Heap::setBackingAllocator(size_t alloc) {
    backing_allocator_ids.push_back(alloc);
    return true;
}

void Heap::printBacking() {
    for(int i = 0; i < backing_allocator_ids.size(); ++i) {
        std::cout << " " << backing_allocator_ids[i];
    }
}