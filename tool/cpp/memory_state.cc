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

bool MemoryState::addHeap(size_t timestamp, const int id, const std::string& type, const std::string& name) {
    if(debug){std::cout << "Wanting to add Heap: " << id << ", " << name << " at: " << timestamp << "\n";}
    Heap h(timestamp, id, type, name);
    auto emp = heaps_.emplace(id, h);
    if(!emp.second) {
        std::cout << "Adding Heap failed: " << "Id: " << id << ", name: " << name << "\n";
        return false;
    }
    num_heaps_added++;
    last_update_ = timestamp;
    return true;
}

bool MemoryState::addCore(size_t timestamp, const int id, const size_t pointer, const size_t size) {
    if(debug){std::cout << "Wanting to add core to: " << id << ", addres start: " <<std::hex << pointer <<std::dec << " size: " << size << " at: " << timestamp << "\n";}


    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " at time: "<< timestamp <<"\n";
        return false;
    }
    if(heap->addCore(timestamp, pointer, size)) {
        num_cores_added++;
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        //heap->printContent();
        return true;
    }
    std::cout << "add core failed at: " << timestamp <<std::endl;
    return false;
}

bool MemoryState::addAllocation(size_t timestamp, const int id, const size_t pointer, const size_t size) {
    if(debug){std::cout << "Wanting to Allocation to: " << id << ", addres start: " <<std::hex << pointer <<std::dec << " size: " << size << " at: " << timestamp << "\n"; }

    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " trying to allocate" <<"\n";
        return false;
    }

    if(heap->addAllocation(timestamp, pointer, size)) {
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        return true;
    }
    std::cout << "Adding Allocation failed(unknown reason): " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << " time: " << timestamp;
    std::cout << " last registerd time was: " << last_update_ << "\n";
    return false;
}

bool MemoryState::removeAllocation(size_t timestamp, const int id, const size_t pointer) {
    if(debug){std::cout << "Wanting to remove allocation from: " << id << ", addres start: " <<std::hex << pointer <<std::dec << " at: " << timestamp << "\n";    }


    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp << " trying to remove allocation" << "\n";
        return false;
    }
    if(heap->removeAllocation(timestamp, pointer)) {
        heap->setLastUpdate(timestamp);
        last_update_ = timestamp;
        return true;
    }
    return false;
}

bool MemoryState::removeCore(size_t timestamp, const int id, const size_t pointer, const size_t size) {
    if(debug){std::cout << "Wanting to remove core from: " << id << ", addres start: " <<std::hex << pointer <<std::dec << " size: " << size << " at: " << timestamp << "\n";    }


    Heap* heap = getHeap(id);
    if(heap == nullptr) {
        std::cout << "Heap: " << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp<< "\n";
    }
    if(heap->removeCore(timestamp, pointer)) {
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

bool MemoryState::removeHeap(size_t timestamp, const int id) {

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
            addAllocation(data->timestamp, (int)data->id, data->pointer, data->size);
            break;
        }
        case Event::Code::HeapFree :
        {
            Event::RemoveAllocation* data = (Event::RemoveAllocation*)event;
            removeAllocation(data->timestamp, (int)data->id, data->pointer);
            break;
        }
        case Event::Code::HeapAddCore :
        {
            Event::AddCore* data = (Event::AddCore*)event;
            addCore(data->timestamp, (int)data->id,data->pointer,data->size);
            break;
        }
        case Event::Code::HeapRemoveCore :
        {
            Event::RemoveCore* data = (Event::RemoveCore*)event;
            removeCore(data->timestamp, (int)data->id,data->pointer,data->size);
            break;
        }
        case Event::Code::HeapCreate :
        {
            Event::AddHeap* data = (Event::AddHeap*)event;
            addHeap(data->timestamp, (int)data->id, data->type, data->name);
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