#ifndef DECODER_H
#define DECODER_H
#include <nan.h>

#include "ringbuffer.h"
#include "memory_state.h"
#include <iostream>
#include <fstream>
#include "./sqlite3/sqlite3.h"

class Decoder : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);

    RingBuffer* getRingbuffer();
    bool decodeValue(uint64_t& ret);
    bool decodeValue(int& ret);
    bool decodeString(std::string& ret);

    bool oneStep(std::stringstream& sql);

    void trySteps();

    MemoryState* memory_state_;
    
    std::vector<Heap*> getMemoryState();
  private:
    explicit Decoder(); //128 * 1024, 0x20000
    ~Decoder();

    void setupEventDatabase();

    bool recording_ = true;
    bool print_error = false;
    bool print_ok = false;
    size_t last_timestamp = 0;
    RingBuffer* ring_;
    sqlite3* event_db_;
    
    unsigned long long registerd_events;

    std::vector<std::pair<char*,size_t>> saved_buffs;

    /* Wrapper functions - start */
    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);
    static NAN_METHOD(UnpackStream);
    static NAN_METHOD(Printas);
    static NAN_METHOD(GetMemoryAsArray);
    static NAN_METHOD(CreateDB);

    enum EventCode
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

    /* Wrapper functions - end */
};

#endif //DECODER_H