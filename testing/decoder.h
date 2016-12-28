#ifndef DECODER_H
#define DECODER_H
#include <nan.h>

class Decoder : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);

  private:
    explicit Decoder(int size = 131072); //128 * 1024, 0x20000
    ~Decoder();

    const int size_;

    /* Wrapper functions - start */
    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);

        /* Debugging methods - start*/
        /* Debugging methods - end*/
    /* Wrapper functions - end */
};

#endif