#ifndef DECODER_H
#define DECODER_H
#include <nan.h>

#include "ringbuffer.h"
#include "memory_state.h"
#include <iostream>
#include <fstream>
#include <vector>

class Decoder : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);

    RingBuffer* getRingbuffer();

    void decodeEvents();
    bool tryGettingEvents(unsigned int number_events);

    void saveBuffer(char* buff, size_t size);
    
    std::vector<Heap*> getMemoryState();
    size_t event_counter_ = 0;
  private:
    explicit Decoder(); //128 * 1024, 0x20000
    ~Decoder();

    bool oneEvent(bool recording = true);
    bool getHeader(int& current_code, size_t& count, size_t& time_stamp);
    bool decodeValue(uint64_t& ret);
    bool decodeValue(int& ret);
    bool decodeString(std::string& ret);

    size_t last_timestamp = 0;
    RingBuffer* ring_;
    MemoryState* memory_state_;

    std::vector<std::pair<char*,size_t>> saved_buffers_;
    

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