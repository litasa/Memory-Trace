#ifndef DECODER_H
#define DECODER_H
#include <nan.h>

#include "ringbuffer.h"
#include "memory_state.h"
#include <iostream>
#include <fstream>

class Decoder : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);

    RingBuffer* getRingbuffer();
    bool decodeValue(uint64_t& ret);
    bool decodeValue(int& ret);
    bool decodeString(std::string& ret);

    void oneStep();

    std::vector<Heap*> getMemoryState();
  private:
    explicit Decoder(); //128 * 1024, 0x20000
    ~Decoder();

    
    size_t last_timestamp = 0;
    RingBuffer* ring_;
    MemoryState* memory_state_;
    unsigned long long registerd_events;

    /* Wrapper functions - start */
    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);
    static NAN_METHOD(UnpackStream);
    static NAN_METHOD(Printas);
    static NAN_METHOD(GetMemoryAsArray);
    static NAN_METHOD(GetNewEvents);

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