#ifndef ALLOCATION_H
#define ALLOCATION_H
#include "memory_object.h"
#include <iostream>
class Allocation : public MemoryObject{
public:
    Allocation::Allocation(size_t pointer, size_t size, size_t timestamp)
    : MemoryObject(pointer, timestamp, size, 0)
    {
        //std::cout << "allocation " << std::hex << pointer_ << std::dec << " with size: " << size << " at: " << timestamp << std::endl;
    }
    void printContent() const {
        std::cout << "\t\t" << std::hex << pointer_ << std::dec << " size: " << used_memory_ << " at timestamp: " << birth_ << "\n";
    }
};
#endif //ALLOCATION_H