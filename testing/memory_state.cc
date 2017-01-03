#include "memory_state.h"

void Core::print() const{
        for ( auto it = allocations_.cbegin(); it != allocations_.cend(); ++it ) {
            std::cout << "\t\t\t" << std::showbase << std::hex << it->first  << std::dec << " : " << it->second.size << "\n";  // cannot modify *it
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
            for(auto it = recently_dead_allocations_.begin(); it != recently_dead_allocations_.end(); ++it) {
                if(it->pointer == pointer) {
                    return &(*it);
                }
            }
            return nullptr;
        }
        return &(found->second);
}

void Core::removeAllocation(Allocation* allocation) {
    recently_dead_allocations_.push_back(*allocation);
    allocations_.erase(allocation->pointer);
}

void Heap::print() const {
    for ( auto it = cores_.cbegin(); it != cores_.cend(); ++it ) {
        std::cout << "\t\t" << std::showbase << std::hex << it->first << std::dec << " : " << it->second.used_memory <<  " // "  << it->second.managed_memory << "\n";  // cannot modify *it
         it->second.print();
     }
}

Core* Heap::getCore(size_t pointer) {
        auto found = cores_.find(pointer);
        if(found == cores_.end()) {
            for(auto it = recently_dead_cores_.begin(); it != recently_dead_cores_.end(); ++it) {
                if(it->pointer == pointer) {
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
                    return &(*it);
                }
            }
            return nullptr;
        }
        return getCore(found->second);
}

void Heap::removeCore(Core* core) {
    if(!core->allocations_.empty() || !core->recently_dead_allocations_.empty()) {
        recently_dead_cores_.push_back(*core);
    }
    cores_.erase(core->pointer);
}

MemoryState::MemoryState() {
};

MemoryState::~MemoryState() {
};

void MemoryState::print(size_t timestamp, int id) const {
        if(id == -1) {
            std::cout << "Printing content of heaps_ at time: " << timestamp << "\n";
            for ( auto it = heaps_.cbegin(); it != heaps_.cend(); ++it ) {
                std::cout << "\t" << it->first << " : " << it->second.name << " used : managed " << it->second.used_memory << " // " << it->second.managed_memory  << "\n";
                it->second.print();
            }
        }
        else {
            std::cout << "Printing content of heaps_[ " << id <<" ] at time: " << timestamp << "\n";
            for ( auto it = heaps_.cbegin(); it != heaps_.cend(); ++it ) {
                if(it->first == id) {
                    std::cout << "\t" << it->first << " : " << it->second.name << " used : managed " << it->second.used_memory << " // " << it->second.managed_memory  << "\n";
                    it->second.print();
                }
            }
        }
}

void MemoryState::addHeap(const int id, const std::string& name, size_t timestamp) {
    // if(timestamp >= 44366723) {
    // std::cout << "Want to add Heap: " << id << ", name: " << name << " at: " << timestamp << std::endl;
    // }
    Heap a;
    a.allocator_id = id;
    a.name = name;
    a.birth = timestamp;
    a.death = -1;
    a.managed_memory = 0;
    a.used_memory = 0;
    auto emp = heaps_.emplace(id, a);
    if(!emp.second) {
        std::cout << "Adding Heap failed: " << "Id: " << id << ", name: " << name << "\n";
        print(timestamp);
    }
}

void MemoryState::addCore(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    // if(timestamp >= 44366723) {
    // std::cout << "Want to add core: " << std::hex << pointer << std::dec << " to heap: " << id << ", size: " << size << " at: " << timestamp << std::endl;
    // }
        auto got = heaps_.find(id);
        if(got == heaps_.end()) {
            std::cout << id << " not found for core: " << std::hex << pointer << std::dec;
        }
        else {
            bool insert = false;
            Core c;
            c.allocator_id = id;
            c.pointer = pointer;
            c.birth = timestamp;
            c.death = -1;
            c.managed_memory = size;
            c.used_memory = 0;
            auto emp = got->second.cores_.insert(std::make_pair(pointer, c));
            if(!emp.second) {
                std::cout << "Adding Core failed: " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " timestamp: " << timestamp  << "\n";
                print(timestamp);
                return;
            }
            heaps_.at(id).managed_memory += size;
        }
    //print(timestamp);
}

void MemoryState::addAllocation(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    // if(timestamp >= 44366723) {
    // std::cout << "Want to add Allocation: " << std::hex << pointer << std::dec << " to heap: " << id << ", size: " << size << " at: " << timestamp << std::endl;
    // }
        Heap* heap = getHeap(id);
        if(heap == nullptr) {
            std::cout << id << " not found for core: " << std::hex << pointer << std::dec;
        }
        else {
            for(auto it = heap->cores_.begin(); it != heap->cores_.end(); ++it) {
                if(it->second.allocationInside(pointer,size)) {
                        Allocation a;
                        a.allocator_id = id;
                        a.pointer = pointer;
                        a.size = size;
                        a.death = -1;
                        a.birth = timestamp;
                        auto emp = it->second.allocations_.insert(std::make_pair(pointer,a));
                        if(!emp.second) {
                            std::cout << "Adding Allocation failed: " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << "\n";
                            std::cout << "trying to allocate in core: " << std::hex << it->second.pointer << std::dec << std::endl;
                            print(timestamp, id);
                            return;
                        }
                        heap->alloc_to_core.emplace(pointer,it->second.pointer);
                        heap->used_memory += size;
                        it->second.used_memory += size;
                        return;
                }
            }
        }
        std::cout << "Adding Allocation failed: " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << "\n";
        print(timestamp, id);
}

void MemoryState::removeAllocation(const int id, const size_t pointer, size_t timestamp) {
    // if(timestamp >= 44366723) {
    // std::cout << "Want to remove Allocation: " << std::hex << pointer << std::dec << " from heap: " << id << " at: " << timestamp << std::endl;
    // }
        Heap* heap = getHeap(id);
        if(heap == nullptr) {
            std::cout << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp;
        }
        else {
            if(id == 0x2200000) {
                std::cout << "heap found" << std::endl;
            }
              Core* core = heap->getCoreForAllocation(pointer);
                if(core != nullptr) {
                    if(id == 0x2200000) {
                std::cout << "core found" << std::endl;
            }
                   Allocation* allocation = core->getAllocation(pointer);
                   if(allocation != nullptr) {
                       if(id == 0x2200000) {
                std::cout << "allocation found" << std::endl;
            }
                    heap->used_memory -= allocation->size;
                    core->removeAllocation(allocation);
                    heap->alloc_to_core.erase(pointer);
                   }
                }
        }
}

void MemoryState::removeCore(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    // if(timestamp >= 44366723) {
    // std::cout << "Want to remove core: " << std::hex << pointer << std::dec << " from heap: " << id << ", size: " << size << " at: " << timestamp << std::endl;
    // }
        Heap* heap = getHeap(id);
        if(heap == nullptr) {
            std::cout << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp;
        }
        else {
            Core* core = heap->getCore(pointer);
            if(core != nullptr) {
                heap->managed_memory -= core->managed_memory;
                core->death = timestamp;
                heap->removeCore(core);
            }
        }
}

void MemoryState::removeHeap(const int id, size_t timestamp) {
    // if(timestamp >= 44366723) {
    //     std::cout << "Want to remove heap: " << id << " at: " << timestamp << std::endl;
    // }
    for (auto it = recently_dead_heaps_.begin(); it != recently_dead_heaps_.end(); ++it) {
        if(it->allocator_id == id) {
            std::cout << "already destroyed: " << id << std::endl;
            return;
        }
    }
    // if(id == 127526) {
    //     print(timestamp, 127526);
    //     std::cout << "not already destroyed..." << std::endl;
    // }
        Heap* heap = getHeap(id);
        if(heap == nullptr) {
            std::cout << "Heap does not exist" << std::endl;
            return;
        }

        if(!heap->cores_.empty() || !heap->recently_dead_cores_.empty()) {
            recently_dead_heaps_.push_back(heaps_.at(id));
        }
        auto num_erased = heaps_.erase(id);
}

Heap* MemoryState::getHeap(const int id) {
        auto found = heaps_.find(id);
        if(found == heaps_.end()) {
            for(auto it = recently_dead_heaps_.begin(); it != recently_dead_heaps_.end(); ++it) {
                if(it->allocator_id == id) {
                    return &(*it);
                }
            }
            return nullptr;
        }
        return &(found->second);
}