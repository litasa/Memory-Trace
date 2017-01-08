#ifndef ALLOCATION_H
#define ALLOCATION_H
class Allocation {
public:
    Allocation::Allocation(size_t pointer, size_t size, size_t timestamp)
    : pointer_(pointer), size_(size), birth_(timestamp), death_(-1), last_update_(timestamp)
    {}

    size_t getPointer() { return pointer_; }
    size_t getPointer() const { return pointer_; }
    size_t getSize() { return size_; }
    size_t getSize() const { return size_; }
    size_t getBirth() { return birth_; }
    size_t getBirth() const { return birth_; }

    size_t getLastUpdate() const { return last_update_; }

    void setLastUpdate(size_t update) { last_update_ = update;}

private:
    size_t pointer_;
    size_t size_;
    size_t birth_;
    size_t death_;
    size_t last_update_;
};
#endif ALLOCATION_H