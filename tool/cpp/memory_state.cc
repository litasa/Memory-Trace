#include "memory_state.h"

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

void MemoryState::printAll() const {
    if(!heaps_.empty())
    {
        std::cout << "\tAlive Content: \n";
        print(0);
    }
    // if(!recently_dead_heaps_.empty()) {
    //     std::cout << "\tDead Content: \n";
    //     for ( auto it = recently_dead_heaps_.cbegin(); it != recently_dead_heaps_.cend(); ++it ) {
    //         std::cout << "\t\t" << it->allocator_id << " : " << it->name << " | B: " << it->birth << ", D: " << it->death <<  "\n";  // cannot modify *it
    //         it->printAll();
    //     }
    // }
}

void MemoryState::printStats() {
    std::cout << "Printing MemoryState stats:";
    std::cout << "\n\tHeaps added: " << num_heaps_added;
    std::cout << "\n\tHeaps removed: " << num_heaps_removed;

    std::cout << "\n\tCores added: " << num_cores_added;
    std::cout << "\n\tCores removed: " << num_cores_removed;

    std::cout << "\n\tAllocations added: " << num_allocations_added;
    std::cout << "\n\tAllocations removed: " << num_allocations_removed;
}

void MemoryState::addHeap(const int id, const std::string& name, size_t timestamp) {
    //std::cout << "Adding Heap\n";
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
        return;
    }
    num_heaps_added++;
}

void MemoryState::addCore(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << "\n";
    }
    else {
        Core c;
        c.allocator_id = id;
        c.pointer = pointer;
        c.birth = timestamp;
        c.death = -1;
        c.managed_memory = size;
        c.used_memory = 0;
        auto emp = heap->cores_.insert(std::make_pair(pointer, c));
        if(!emp.second) {
            std::cout << "Adding Core failed: " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " timestamp: " << timestamp  << "\n";
            return;
        }
        heaps_.at(id).managed_memory += size;
        num_cores_added++;
    }
}

void MemoryState::addAllocation(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    //std::cout << "Adding allocation\n";
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << "\n";
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
                    std::cout << "Adding Allocation failed (already exists): " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " created: " << timestamp << "\n";
                    std::cout << "had values: " << emp.first->second.allocator_id << " pointer: " << std::hex << emp.first->second.pointer << std::dec << " size: " << emp.first->second.size << " created: " << emp.first->second.birth << "\n";
                    return;
                }
                heap->alloc_to_core.emplace(pointer,it->second.pointer);
                heap->used_memory += size;
                it->second.used_memory += size;
                num_allocations_added++;
                return;
            }
        }
    }
    std::cout << "Adding Allocation failed(unknown reason): " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " time: " << timestamp  <<"\n";
    //print(timestamp);
}

void MemoryState::removeAllocation(const int id, const size_t pointer, size_t timestamp) {
    //std::cout << "Removing Allocation\n";
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp << "\n";
    }
    else {
        Core* core = heap->getCoreForAllocation(pointer);
        if(core != nullptr) {
            Allocation* allocation = core->getAllocation(pointer);
            if(allocation != nullptr) {
                heap->used_memory -= allocation->size;
                core->removeAllocation(allocation);
                heap->alloc_to_core.erase(pointer);
                num_allocations_removed++;
            }
        }
    }
}

void MemoryState::removeCore(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    //std::cout << "Removing Core\n";
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp<< "\n";
    }
    else {
        Core* core = heap->getCore(pointer);
        if(core != nullptr) {
            heap->managed_memory -= core->managed_memory;
            core->death = timestamp;
            heap->removeCore(core);
            num_cores_removed++;
        }
    }
}

void MemoryState::removeHeap(const int id, size_t timestamp) {
    //std::cout << "Removing Heap\n";
    for (auto it = recently_dead_heaps_.begin(); it != recently_dead_heaps_.end(); ++it) {
        if(it->allocator_id == id) {
            std::cout << "already destroyed: " << id << "\n";
            return;
        }
    }
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Removing non existing heap: " << id << ", at time: " << timestamp << "\n";
        return;
    }

    if(!heap->cores_.empty() || !heap->recently_dead_cores_.empty()) {
        recently_dead_heaps_.push_back(heaps_.at(id));
    }
    num_heaps_removed++;
    auto num_erased = heaps_.erase(id);
}

Heap* MemoryState::getHeap(const int id) {
    auto found = heaps_.find(id);
    if(found == heaps_.end()) {
        for(auto it = recently_dead_heaps_.begin(); it != recently_dead_heaps_.end(); ++it) {
            if(it->allocator_id == id) {
                std::cout << "getting dead heap: " << id << "\n";
                return &(*it);
            }
        }
        return nullptr;
    }
    return &(found->second);
}

std::vector<Heap*> MemoryState::getHeaps() {
    std::vector<Heap*> ret;
    for(auto it = heaps_.begin(); it != heaps_.end(); ++it) {
        ret.push_back(&it->second);
    }
    return ret;
}