#ifndef ALLOCATION_H
#define ALLOCATION_H
#include <stdlib.h> //size_t
class Allocation {
    public:
    int allocator_id;
    size_t pointer;
    size_t size;
    size_t birth;
    size_t death;
};
#endif //ALLOCATION_H
