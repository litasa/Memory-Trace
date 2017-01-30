#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <sstream>

namespace Event 
{
    enum Code
        {
            BeginStream     = 1,
            EndStream,

            HeapCreate = 10,
            HeapDestroy,

            HeapAddCore,
            HeapRemoveCore,

            HeapAllocate,
            HeapFree,

            EventStart = 20,
            EventEnd,

            /*Stingray specifics from here on*/
            SetBackingAllocator = 30,
        };

    struct Event {
        Event();
        Event(uint64_t eventNumber, size_t eventType, uint64_t timestamp)
        : eventNumber(eventNumber), eventType(eventType), timestamp(timestamp) {};
        ~Event() {};

        virtual void getAsCSV(std::stringstream& ss) {
		    ss << eventNumber << "," << eventType << "," << timestamp;
	    }

	    virtual void getAsVerbose(std::stringstream& ss) = 0;

        uint64_t eventNumber;
        size_t eventType;
        uint64_t timestamp;
    };

    struct InitStream : public Event {
        InitStream(uint64_t eventNumber, size_t eventType, uint64_t timestamp, uint64_t stream_magic, std::string& platform, uint64_t frequency)
        : Event(eventNumber, eventType, timestamp), stream_magic(stream_magic), platform(platform), system_frequency(system_frequency)
        { };
        ~InitStream();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << stream_magic << "," << platform << "," << system_frequency << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "InitStream " << "at time: " << timestamp;
            ss << "\n\tstream magic: " << std::showbase << std::hex << stream_magic << std::dec;
            ss << "\n\tplatform: " << platform;
            ss << "\n\tsystem frequency: " << system_frequency;
            ss << "\n";
        }

        uint64_t stream_magic;
        std::string platform;
        uint64_t system_frequency;
    };

    struct StopStream : public Event {
        StopStream(uint64_t eventNumber, size_t eventType, uint64_t timestamp)
        : Event(eventNumber, eventType, timestamp)
        { };
        ~StopStream();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
            ss << ", #stream ended";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "StopStream " << "at time: " << timestamp;
        }
    };

    struct AddHeap : public Event {
        AddHeap(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id, std::string& type, std::string& name)
        : Event(eventNumber, eventType, time)
        , id(id)
        , type(type)
        , name(name)
        {};
        ~AddHeap();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << type << "," << name << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "AddHeap " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\ttype: " << type;
            ss << "\n\tname: " << name;
            ss << "\n";
        }

        uint64_t id;
        std::string type;
        std::string name;
    };

    struct RemoveHeap : public Event {
        RemoveHeap(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id)
        : Event(eventNumber, eventType, time), id(id) {};
        ~RemoveHeap();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "RemoveHeap " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n";
        }
        uint64_t id;
    };

    struct AddCore : public Event {
        AddCore(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~AddCore();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << pointer << "," << size << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "AddCore " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n\tsize: " << size;
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct RemoveCore : public Event {
        RemoveCore(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~RemoveCore();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << pointer << "," << size << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "RemoveCore " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n\tsize: " << size;
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct AddAllocation : public Event {
        AddAllocation(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer,uint64_t size_bytes)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer), size(size_bytes) { };
        ~AddAllocation();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << pointer << "," << size << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "AddAllocation " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n\tsize: " << size;
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
        uint64_t size;
    };

    struct RemoveAllocation : public Event {
        RemoveAllocation(uint64_t eventNumber, size_t eventType, uint64_t time, uint64_t id,uint64_t pointer)
        : Event(eventNumber, eventType, time), id(id), pointer(pointer) { };
        ~RemoveAllocation();

        virtual void getAsCSV(std::stringstream& ss) override {
		    Event::getAsCSV(ss);
		    ss << "," << id << "," << pointer << "\n";
	    }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "RemoveAllocation " << "at time: " << timestamp;
            ss << "\n\tid: " << id;
            ss << "\n\tpointer: " << std::showbase << std::hex << pointer << std::dec;
            ss << "\n";
        }

        uint64_t id;
        uint64_t pointer;
    };

    struct StartEvent : public Event {
        StartEvent(uint64_t eventNumber, size_t eventType, uint64_t time, std::string& name)
        : Event(eventNumber, eventType, time), name(name) { };
        ~StartEvent();

        virtual void getAsCSV(std::stringstream& ss) override {
            Event::getAsCSV(ss);
            ss << "," << name << "\n";
        }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "StartEvent" << "at time: " << timestamp;
            ss << "\n\tname: " << name << "\n";
        }

        std::string name;
    };

    struct EndEvent : public Event {
        EndEvent(uint64_t eventNumber, size_t eventType, uint64_t time, std::string& name)
        : Event(eventNumber, eventType, time), name(name) { };
        ~EndEvent();

        virtual void getAsCSV(std::stringstream& ss) override {
            Event::getAsCSV(ss);
            ss << "," << name << "\n";
        }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "EndEvent" << "at time: " << timestamp;
            ss << "\n\tname: " << name << "\n";
        }

        std::string name;
    };

    struct HeapSetBackingAllocator : public Event {
        HeapSetBackingAllocator(uint64_t eventNumber, size_t eventType, uint64_t time, size_t for_heap, size_t set_to_heap)
        : Event(eventNumber, eventType, time), for_heap(for_heap), set_to_heap(set_to_heap) { };
        ~HeapSetBackingAllocator();

        virtual void getAsCSV(std::stringstream& ss) override {
            Event::getAsCSV(ss);
            ss << "," << for_heap << "," << set_to_heap << "\n";
        }

        virtual void getAsVerbose(std::stringstream& ss) override {
            ss << "(" << eventNumber << ")" << "HeapSetBackingAllocator" << "at time: " << timestamp;
            ss << "\n\tfor heap: " << for_heap;
            ss << "\n\tset to heap: " << set_to_heap << "\n";
        }

        size_t for_heap;
        size_t set_to_heap;
    };
}
#endif //EVENT_H