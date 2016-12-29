#ifndef DECODER_H
#define DECODER_H
#include <nan.h>

#include "ringbuffer.h"

class Decoder : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);

    RingBuffer* getRingbuffer();
    bool decodeValue(uint64_t& ret);
    bool decodeString(std::string& ret);
  private:
    explicit Decoder(); //128 * 1024, 0x20000
    ~Decoder();

    
    

    RingBuffer* ring_;

    /* Wrapper functions - start */
    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);
    static NAN_METHOD(UnpackStream);

        /* Debugging methods - start*/
        /* Debugging methods - end*/
    /* Wrapper functions - end */
};

#endif