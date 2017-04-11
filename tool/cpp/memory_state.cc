#include "memory_state.h"

MemoryState::MemoryState() {
};

MemoryState::~MemoryState() {
};

void MemoryState::setInits(size_t stream_magic, std::string platform, size_t frequency) {
    stream_magic_ = stream_magic;
    platform_ = platform;
    frequency_ = (double)(frequency);
}

bool MemoryState::addHeap(size_t timestamp, const uint64_t id, const std::string& type, const std::string& name) {
    Heap h(timestamp, id, type, name);
    auto emp = heaps_.emplace(id, h);
    if(!emp.second) {
        if(debug) { std::cout << "Adding Heap failed: " << "Id: " << id << ", name: " << name << "\n"; }
        return false;
    }
    num_heaps_added++;
    last_update_ = timestamp;
    return true;
}

bool MemoryState::addCore(size_t timestamp, const uint64_t id, const size_t pointer, const size_t size) {
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        if(debug) {std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " at time: "<< timestamp <<"\n";}
        return false;
    }
    if(heap->addCore(timestamp, pointer, size)) {
        num_cores_added++;
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        return true;
    }
    return false;
}

bool MemoryState::addAllocation(size_t timestamp, const uint64_t id, const size_t pointer, const size_t size, bool core_exist) {
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        if(debug) {std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " trying to allocate" <<"\n";}
        return false;
    }

    if(heap->addAllocation(timestamp, pointer, size, core_exist)) {
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        return true;
    }
    if(debug) {std::cout << "Adding Allocation failed(unknown reason): " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " time: " << timestamp;}
    return false;
}

bool MemoryState::growCore(size_t timestamp, const size_t id, const size_t pointer, const size_t size) {
        Heap* heap = getHeap(id);
    if(heap == nullptr) {
        return false;
    }
    heap->growCore(timestamp,pointer,size);
    return true;
}

bool MemoryState::removeAllocation(size_t timestamp, const uint64_t id, const size_t pointer, bool core_exist) {
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        if(debug) {std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp << " trying to remove allocation" << "\n";}
        return false;
    }
    if(heap->removeAllocation(timestamp, pointer, core_exist)) {
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        return true;
    }
    return false;
}

bool MemoryState::removeAllAllocations(size_t timestamp, const uint64_t id) {
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        if(debug) {std::cout << "Heap: " << id << " not found trying to remove all allocations at time: " << timestamp<< "\n";}
        return false;
    }
    for(auto it = heap->cores_.begin(); it != heap->cores_.end(); it++) {
        it->second.removeAllAllocations();
    }
    return true;
}

bool MemoryState::removeCore(size_t timestamp, const uint64_t id, const size_t pointer, const size_t size) {
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        if(debug) {std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp<< "\n";}
        return false;
    }
    if(heap->removeCore(timestamp, pointer)) {
        heap->setLastUpdate(timestamp);
        num_cores_removed++;
        last_update_ = timestamp;
        return true;
    }
    return false;
}

bool MemoryState::shrinkCore(size_t timestamp, const size_t id, const size_t pointer, const size_t size) {
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        return false;
    }
    heap->shrinkCore(timestamp,pointer,size);
    return true;
}

bool MemoryState::removeHeap(size_t timestamp, const uint64_t id) {
    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        if(debug) {std::cout << "Removing non existing heap: " << id << ", at time: " << timestamp << "\n";}
        return false;
    }
    heap->death_ = timestamp;
    return true;
}

Heap* MemoryState::getHeap(const uint64_t id) {
    auto found = heaps_.find(id);
    if(found == heaps_.end()) {
        if(id == -1) {
            return nullptr;
        }
        if(debug) {std::cout << "No heap Found, " << id << std::endl;}
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

void MemoryState::addEvent(Event::Event* event) {
    switch(event->eventType) {

        case Event::Code::HeapAllocate :
        {
            Event::AddAllocation* data = (Event::AddAllocation*)event;
            addAllocation(data->timestamp, data->id, data->pointer, data->size, data->owned);
            break;
        }

        case Event::Code::HeapFree :
        {
            Event::RemoveAllocation* data = (Event::RemoveAllocation*)event;
            removeAllocation(data->timestamp, data->id, data->pointer, data->owned);
            break;
        }

        case Event::Code::HeapAddCore :
        {
            Event::AddCore* data = (Event::AddCore*)event;
            addCore(data->timestamp, data->id,data->pointer,data->size);
            break;
        }

        case Event::Code::HeapRemoveCore :
        {
            Event::RemoveCore* data = (Event::RemoveCore*)event;
            removeCore(data->timestamp, data->id,data->pointer,data->size);
            break;
        }

        case Event::Code::HeapGrowCore :
        {
            Event::GrowCore* data = (Event::GrowCore*)event;
            growCore(data->timestamp, data->id, data->pointer, data->size);
            break;
        }
        case Event::Code::HeapCreate :
        {
            Event::AddHeap* data = (Event::AddHeap*)event;
            addHeap(data->timestamp, data->id, data->type, data->name);
            break;
        }
        case Event::Code::HeapDestroy :
        {
            Event::RemoveHeap* data = (Event::RemoveHeap*)event;
            removeHeap(data->timestamp, (int)data->id);
            break;
        }
        case Event::Code::SetBackingAllocator :
        {
            Event::HeapSetBackingAllocator* data = (Event::HeapSetBackingAllocator*)event;
            getHeap(data->for_heap)->setBackingAllocator(data->set_to_heap);
            break;
        }
        case Event::Code::BeginStream :
        {
            Event::InitStream* data = (Event::InitStream*)event;
            setInits(data->stream_magic, data->platform, data->system_frequency);
            break;
        }

        case Event::Code::HeapFreeAll :
        {
            Event::RemoveAllAllocations* data = (Event::RemoveAllAllocations*)event;
            removeAllAllocations(data->timestamp,data->id);
            break;
        }

        case Event::Code::EventStart :
        {
            //std::cout << "Event: " << ((Event::StartEvent*)event)->name << " started" << std::endl;
            break;
        }

        case Event::Code::EventEnd :
        {
            //std::cout << "Event: " << ((Event::EndEvent*)event)->name << " ended" << std::endl;            
            break;
        }

        case Event::Code::EndStream :
        {
            break;
        }

        default :
        {
            std::cout << "event not handled by memory: " << event->eventType << std::endl;
            break;
        }
    }
    last_update_ = event->timestamp;
    //eventList.push_back(event);
}