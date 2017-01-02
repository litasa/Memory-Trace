#include "memory_state.h"

void Core::print() const{
        //"\tPrinting content of Cores_: " << pointer << "\n";
        for ( auto it = allocations_.cbegin(); it != allocations_.cend(); ++it ) {
            std::cout << "\t\t\t" << std::hex << it->first  << std::dec << " : " << it->second.size << "\n";  // cannot modify *it
        }    
}

void Allocator::print() const {
    //"Printing content of Allocator: " << allocator_id << "\n";
    for ( auto it = cores_.cbegin(); it != cores_.cend(); ++it ) {
        std::cout << "\t\t" << std::showbase << std::hex << it->first << std::dec << " : " << it->second.size << "\n";  // cannot modify *it
         it->second.print();
     }
}

MemoryState::MemoryState() {
    times_called = 0;
};

MemoryState::~MemoryState() {
    
};

void MemoryState::print(size_t timestamp) const {
        std::cout << "Printing content of Allocators_ at time: " << timestamp << "\n";
        for ( auto it = allocators_.cbegin(); it != allocators_.cend(); ++it ) {
            std::cout << "\t" << it->first << " : " << it->second.name << "\n";  // cannot modify *it
            it->second.print();
        }
}

void MemoryState::addHeap(const int id, const std::string& name, size_t timestamp) {
    times_called++;
    Allocator a;
    a.allocator_id = id;
    a.name = name;
    a.time_stamp = timestamp;
    auto emp = allocators_.emplace(id, a);
    
    if(id == 246) {
        print(timestamp);
        std::cout << "Id: " << 246 << "added at timestamp: " << timestamp << "times called: " << times_called << "\n";
    }
    if(!emp.second) {
        std::cout << "Adding Heap failed: " << "Id: " << id << ", name: " << name << "times called: " << times_called  << "\n";
        print(timestamp);
    }
    //print(timestamp);
}

void MemoryState::addCore(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    auto got = allocators_.find(id);
    if(got == allocators_.end()) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec;
    }
    else {
        Core c;
        c.allocator_id = id;
        c.pointer = pointer;
        c.size = size;
        c.time_stamp = timestamp;
        auto emp = got->second.cores_.emplace(pointer, c);
        if(!emp.second) {
            std::cout << "Adding Core failed: " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << "\n";
        }
    }
    //print(timestamp);
}

void MemoryState::addAllocation(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    auto got = allocators_.find(id);
    if(got == allocators_.end()) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec;
    }
    else {
        auto at = got->second.cores_.begin();
        for(auto it = got->second.cores_.begin(); it != got->second.cores_.end(); ++it) {
            if(it->second.pointer <= pointer && it->second.pointer + it->second.size >= pointer) {
                    Allocation a;
                    a.allocator_id = id;
                    a.pointer = pointer;
                    a.size = size;
                    a.time_stamp = timestamp;
                    auto emp = it->second.allocations_.emplace(pointer,a);
                    if(!emp.second) {
                        std::cout << "Adding Allocation failed: " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << "\n";
                    }
                    got->second.alloc_to_core.emplace(pointer,it->second.pointer);
                    //print(timestamp);
                    return;
            }
        }
    }
    std::cout << "Adding Allocation failed: " << "Id: " << id << ", pointer: " << std::hex << pointer << std::dec << " size: " << size << "\n";
    //print(timestamp);
}

void MemoryState::removeAllocation(const int id, const size_t pointer, size_t timestamp) {
    auto got = allocators_.find(id);
    if(got == allocators_.end()) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp;
    }
    else {
        got->second.cores_[got->second.alloc_to_core[pointer]].allocations_.erase(pointer);
        //print(timestamp);
    }
}

void MemoryState::removeCore(const int id, const size_t pointer, const size_t size, size_t timestamp) {
    auto got = allocators_.find(id);
    if(got == allocators_.end()) {
        std::cout << id << " not found for core: " << std::hex << pointer << std::dec << " at time: " << timestamp;
    }
    else {
        got->second.cores_.erase(pointer);
        //print(timestamp);
    }
}

void MemoryState::removeHeap(const int id, size_t timestamp) {
    if(id == 246) {
        std::cout << "Id: " << 246 << "removed at timestamp: " << timestamp << "\n";
    }
    allocators_.erase(id);
    //print(timestamp);
}