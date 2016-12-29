#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include <nan.h>
#include <string>
class RingBuffer : public Nan::ObjectWrap {
  public:
    static NAN_MODULE_INIT(Init);
    explicit RingBuffer(int size_ = 131072); //128 * 1024, 0x20000
    ~RingBuffer();
    size_t getNumUnread();
    uint8_t readNext();
    size_t populate(char* buffer, size_t size);
    void setRollback();
    void doRollback();

    std::string extractString(size_t length);
  private:
    

    
    size_t getWritePos();
    
    

    char* buffer_;
    const int size_;
    unsigned int read_position_;
    unsigned int rollback_;
    size_t unread_;
    size_t ring_mask_;

    /* Wrapper functions - start */
    static Nan::Persistent<v8::Function> constructor;

    static NAN_METHOD(New);
    static NAN_METHOD(Populate);
    static NAN_METHOD(ReadNext);
    static NAN_METHOD(SetRollback);
    static NAN_METHOD(DoRollback);

        /* Debugging methods - start*/
        static NAN_METHOD(GetBuffer);
        static NAN_METHOD(GetReadPosition);
        static NAN_METHOD(GetUnread);
        static NAN_METHOD(GetWritePosition);
        static NAN_METHOD(GetRollbackPosition);
        /* Debugging methods - end*/
    /* Wrapper functions - end */
};

#endif