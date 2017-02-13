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
    bool decodeString(std::string& ret);
    bool decodeBool(bool& ret);
    bool decodeHeader(std::stringstream& ss,uint64_t& count, uint64_t& current_code, uint64_t& time_stamp, uint64_t& thread_id);

    Event::Event* oneStep(bool save_to_file = true);

    bool print_ok();
    bool print_error();

    void saveToFile(bool save, Event::Event* event);

    void trySteps();

    MemoryState* memory_state_;
    
    std::vector<Heap*> getMemoryState();
  private:
    explicit Decoder();
    ~Decoder();
    std::ofstream outfile;
    const char* filename = "test.csv";
    bool _stream_end = false;
    //bool print_ok = false;
    size_t last_timestamp = 0;
    RingBuffer* ring_;
    
    unsigned long long registerd_events;

    std::vector<std::pair<char*,size_t>> saved_buffs;

    /* Wrapper functions - start */
    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);
    static NAN_METHOD(UnpackStream);
    static NAN_METHOD(Printas);
    static NAN_METHOD(GetMemoryAsArray);
    static NAN_METHOD(GetDeadHeaps);
    static NAN_METHOD(GetFilteredData);    
    static NAN_METHOD(StreamEnd);

    /* Wrapper functions - end */
};

#endif //DECODER_H