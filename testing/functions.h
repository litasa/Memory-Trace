#ifndef NATIVE_EXTENSION_GRAB_H
#define NATIVE_EXTENSION_GRAB_H

#include <nan.h>

// Example top-level functions. These functions demonstrate how to return various js types.
// Implementations are in functions.cc

NAN_METHOD(DecodeValue);
NAN_METHOD(DecodeString);
NAN_METHOD(EncodeString);
NAN_METHOD(EncodeValue);
NAN_METHOD(EncodeNumber);
NAN_METHOD(DecodeStream);

class RingBuffer : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);

  private:
    explicit RingBuffer(int size_ = 131072); //128 * 1024, 0x20000
    ~RingBuffer();

    uint8_t getByte();
    size_t getWritePos();
    size_t populate(char* buffer, size_t size);
    void setRollback();
    void doRollback();

    static NAN_METHOD(New);
    static NAN_METHOD(Populate);
    static NAN_METHOD(ReadByte);
    static NAN_METHOD(SetRollback);
    static NAN_METHOD(DoRollback);

    /*Debugging methods - start*/
    static NAN_METHOD(GetBuffer);
    static NAN_METHOD(GetReadPosition);
    static NAN_METHOD(GetUnread);
    static NAN_METHOD(GetWritePosition);
    static NAN_METHOD(GetRollbackPosition);
    /*Debugging methods - end*/
    static Nan::Persistent<v8::Function> constructor;
    char* buffer_;
    const int size_;
    unsigned int read_position_;
    unsigned int rollback_;
    int unread_;
    int ring_mask_;
};
#endif
