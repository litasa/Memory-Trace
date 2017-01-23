#include "memory_state.h"

MemoryState::MemoryState() {
};

MemoryState::~MemoryState() {
    for(auto &it:eventList)
    {
        delete it;
    }
    eventList.clear();
};

void MemoryState::setInits(size_t stream_magic, std::string platform, size_t frequency) {
    stream_magic_ = stream_magic;
    platform_ = platform;
    frequency_ = 1/(double)frequency;
}

bool MemoryState::addHeap(const int id, const std::string& type, const std::string& name, size_t timestamp) {
    if(debug){std::cout << "Wanting to add Heap: " << id << ", " << name << " at: " << timestamp << "\n";}
    Heap h(id, type, name, timestamp);
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
    if(debug){std::cout << "Wanting to add core to: " << id << ", addres start: " <<std::hex << pointer <<std::dec << " size: " << size << " at: " << timestamp << "\n";}


    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " at time: "<< timestamp <<"\n";
        return false;
    }
    if(heap->addCore(pointer, size, timestamp)) {
        num_cores_added++;
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        //heap->printContent();
        return true;
    }
    std::cout << "add core failed at: " << timestamp <<std::endl;
    return false;
}

bool MemoryState::addAllocation(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    if(debug){std::cout << "Wanting to Allocation to: " << id << ", addres start: " <<std::hex << pointer <<std::dec << " size: " << size << " at: " << timestamp << "\n"; }

    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " trying to allocate" <<"\n";
        return false;
    }

    if(heap->addAllocation(pointer, size, timestamp)) {
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        return true;
    }
    std::cout << "Adding Allocation failed(unknown reason): " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " time: " << timestamp;
    std::cout << " last registerd time was: " << last_update_ << "\n";
    return false;
}

bool MemoryState::removeAllocation(const int id, const size_t pointer, size_t timestamp) {
    if(debug){std::cout << "Wanting to remove allocation from: " << id << ", addres start: " <<std::hex << pointer <<std::dec << " at: " << timestamp << "\n";    }


    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp << " trying to remove allocation" << "\n";
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
    if(debug){std::cout << "Wanting to remove core from: " << id << ", addres start: " <<std::hex << pointer <<std::dec << " size: " << size << " at: " << timestamp << "\n";    }


    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp<< "\n";
    }
    if(heap->removeCore(pointer, timestamp)) {
        heap->setLastUpdate(timestamp);
        num_cores_removed++;
        last_update_ = timestamp;
        return true;
    }
    //std::cout << "remove core failed at " << timestamp << "\n";
    //std::cout << "\ttrying to remove from: " << id << " heap: " << std::hex << pointer << std::dec << " size: " << size << "\n";
    //heap->printContent();
    return false;
}

bool MemoryState::removeHeap(const int id, size_t timestamp) {

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
        heap->dead = true;
        return true;
    }
    std::cout << "remove heap failed at: " << timestamp << std::endl;
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

void MemoryState::addEvent(Event::Event* event) {
    switch(event->eventType) {
        case Event::Code::HeapAllocate :
        {
            Event::AddAllocation* data = (Event::AddAllocation*)event;
            addAllocation((int)data->id, data->pointer, data->size, data->timestamp);
            break;
        }
        case Event::Code::HeapFree :
        {
            Event::RemoveAllocation* data = (Event::RemoveAllocation*)event;
            removeAllocation((int)data->id, data->pointer, data->timestamp);
            break;
        }
        case Event::Code::HeapAddCore :
        {
            Event::AddCore* data = (Event::AddCore*)event;
            addCore((int)data->id,data->pointer,data->size, data->timestamp);
            break;
        }
        case Event::Code::HeapRemoveCore :
        {
            Event::RemoveCore* data = (Event::RemoveCore*)event;
            removeCore((int)data->id,data->pointer,data->size,data->timestamp);
            break;
        }
        case Event::Code::HeapCreate :
        {
            Event::AddHeap* data = (Event::AddHeap*)event;
            addHeap((int)data->id, data->type, data->name, data->timestamp);
            break;
        }
        case Event::Code::HeapDestroy :
        {
            Event::RemoveHeap* data = (Event::RemoveHeap*)event;
            removeHeap((int)data->id, data->timestamp);
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

        case Event::Code::EventStart :
        {
            std::cout << "Event: " << ((Event::StartEvent*)event)->name << " started" << std::endl;
            break;
        }

        case Event::Code::EventEnd :
        {
            std::cout << "Event: " << ((Event::EndEvent*)event)->name << " ended" << std::endl;            
            break;
        }
        default :
        {
            std::cout << "event not handled by memory: " << event->eventType << std::endl;
            break;
        }
    }
    eventList.push_back(event);
}