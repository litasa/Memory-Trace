#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include <nan.h>
#include <string>
class RingBuffer {
  public:
    explicit RingBuffer(int size_ = 128); //128 * 1024, 0x20000
    ~RingBuffer();
    size_t getNumUnread();
    size_t getReadPosition(); 
    size_t getWritePosition();
    size_t getNumProcessed();
    uint8_t readNext();
    size_t populate(char* buff, size_t size, size_t exclude_from_front = 0);
    void setRollback();
    void doRollback();

    std::string extractString(size_t length);
  private:

    char* buffer_;
    const size_t size_;
    size_t read_position_;
    size_t write_position_;
    size_t rollback_;
    size_t data_processed_;
    size_t unread_;
    size_t ring_mask_;
};

#endif