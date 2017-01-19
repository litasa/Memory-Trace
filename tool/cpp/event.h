#ifndef EVENT_H
#define EVENT_H

#include <string>

namespace Event 
{
    enum Code
        {
        BeginStream     = 1,
        EndStream,

        HeapCreate = 18,
        HeapDestroy,

        HeapAddCore,
        HeapRemoveCore,

        HeapAllocate,
        HeapFree,
        };

    struct Event {
        Event();
        Event(uint64_t eventNumber, unsigned int eventType, uint64_t timestamp)
        : eventNumber(eventNumber), eventType(eventType), timestamp(timestamp) {};
        ~Event() {};

        uint64_t eventNumber;
        unsigned int eventType;
        uint64_t timestamp;
    };

    struct InitStream : public Event {
        InitStream(uint64_t eventNumber, unsigned int eventType, uint64_t timestamp, uint64_t stream_magic, std::string& platform, uint64_t frequency)
        : Event(eventNumber, eventType, timestamp), stream_magic(stream_magic), platform(platform), system_frequency(system_frequency)
        { };
        ~InitStream();

        uint64_t stream_magic;
        std::string platform;
        uint64_t system_frequency;
    };

    struct StopStream : public Event {
        StopStream(uint64_t eventNumber, unsigned int eventType, uint64_t timestamp)
        : Event(eventNumber, eventType, timestamp)
        { };
        ~StopStream();
    };

    struct AddHeap : public Event {
        AddHeap(uint64_t eventNumber, unsigned int eventType, uint64_t time, uint64_t id, std::string& name)
        : Event(eventNumber, eventType, time)
        , id(id)
        , name(name)
        {};
        ~AddHeap();

        uint64_t id;
        std::string name;
    };

    struct RemoveHeap : public Event {
        RemoveHeap(uint64_t eventNumber, unsigned int eventType, uint64_t time, uint64_t id)
        : Event(eventNumber, eventType, time), id(id) {};
        ~RemoveHeap();

        uint64_t id;
    };

    struct AddCore : public Event {
        AddCore(uint64_t eventNumber, unsigned int eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~AddCore();

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct RemoveCore : public Event {
        RemoveCore(uint64_t eventNumber, unsigned int eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~RemoveCore();

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct AddAllocation : public Event {
        AddAllocation(uint64_t eventNumber, unsigned int eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~AddAllocation();

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct RemoveAllocation : public Event {
        RemoveAllocation(uint64_t eventNumber, unsigned int eventType, uint64_t time, uint64_t id,uint64_t pointer)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer) { };
        ~RemoveAllocation();

        uint64_t id;
        uint64_t pointer;
    };
}
#endif //EVENT_H