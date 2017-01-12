#ifndef MEMORY_EVENT_H
#define MEMORY_EVENT_H

class MemoryObject {
public:
    MemoryObject(size_t pointer, size_t timestamp, size_t used_size, size_t managed_size) 
    : pointer_(pointer), birth_(timestamp), death_(-1), last_update_(timestamp), used_memory_(used_size), managed_memory_(managed_size)
    { }
    

    size_t getPointer() { return pointer_; }
    size_t getPointer() const { return pointer_; }

    size_t getUsedMemory() { return used_memory_; }
    size_t getUsedMemory() const { return used_memory_; }

    size_t getManagedMemory() { return managed_memory_; }
    size_t getManagedMemory() const { return managed_memory_; }

    size_t getBirth() { return birth_; }
    size_t getBirth() const { return birth_; }

    size_t getLastUpdate() { return last_update_; }
    size_t getLastUpdate() const { return last_update_; }
    void   setLastUpdate(size_t time) { last_update_ = time;}

    size_t pointer_;
    size_t birth_;
    size_t death_;
    size_t last_update_;
    size_t used_memory_;
    size_t managed_memory_;
protected:
   
private:

};

#endif //MEMORY_EVENT_H