#include "memory_state.h"

MemoryState::MemoryState() {
};

MemoryState::~MemoryState() {
};

void MemoryState::setInits(size_t stream_magic, std::string platform, size_t frequency) {
    stream_magic_ = stream_magic;
    platform_ = platform;
    frequency_ = 1/(double)frequency;
    std::cout << "frequencies" << frequency_ << " " << frequency << std::endl;
}

bool MemoryState::addHeap(const int id, const std::string& name, size_t timestamp) {
    if(last_update_ > timestamp) {
        std::cout << "We have an error: " << last_update_ <<  " > " << timestamp << std::endl;
    }
    Heap h(id, name, timestamp);
    auto emp = heaps_.emplace(id, h);
    if(!emp.second) {
        std::cout << "Adding Heap failed: " << "Id: " << id << ", name: " << name << "\n";
        return false;
    }
    num_heaps_added++;
    last_update_ = timestamp;
    return true;
}

bool MemoryState::addCore(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    if(last_update_ > timestamp) {
        std::cout << "We have an error: " << last_update_ <<  " > " << timestamp << std::endl;
    }
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << "\n";
        return false;
    }
    if(heap->addCore(pointer, size, timestamp)) {
        num_cores_added++;
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        return true;
    }
    return false;
}

bool MemoryState::addAllocation(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    if(last_update_ > timestamp) {
        std::cout << "We have an error: " << last_update_ <<  " > " << timestamp << std::endl;
    }
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << "\n";
        return false;
    }

    if(heap->addAllocation(pointer, size, timestamp)) {
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        return true;
    }
    std::cout << "Adding Allocation failed(unknown reason): " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " time: " << timestamp  <<"\n";
    return false;
}

bool MemoryState::removeAllocation(const int id, const size_t pointer, size_t timestamp) {
    if(last_update_ > timestamp) {
        std::cout << "We have an error: " << last_update_ <<  " > " << timestamp << std::endl;
    }
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp << "\n";
        return false;
    }
    if(heap->removeAllocation(pointer, timestamp)) {
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        return true;
    }
    return false;
}

bool MemoryState::removeCore(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    if(last_update_ > timestamp) {
        std::cout << "We have an error: " << last_update_ <<  " > " << timestamp << std::endl;
    }
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp<< "\n";
    }
    if(heap->removeCore(pointer, timestamp)) {
        heap->setLastUpdate(timestamp);
        num_cores_removed++;
        last_update_ = timestamp;
        return true;
    }
    return false;
}

bool MemoryState::removeHeap(const int id, size_t timestamp) {
    if(last_update_ > timestamp) {
        std::cout << "We have an error: " << last_update_ <<  " > " << timestamp << std::endl;
    }
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Removing non existing heap: " << id << ", at time: " << timestamp << "\n";
        return false;
    }
    size_t items_removed = heaps_.erase(id);
    if(items_removed == 1) {
        heap->setLastUpdate(timestamp);
        num_heaps_removed++;
        last_update_ = timestamp;
        return true;
    }
    return false;
}

Heap* MemoryState::getHeap(const int id) {
    auto found = heaps_.find(id);
    if(found == heaps_.end()) {
        std::cout << "No heap Found, " << id << std::endl;
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