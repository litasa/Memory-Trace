#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include <nan.h>
#include <string>
class RingBuffer {
  public:
    explicit RingBuffer(int size_ = 128); //128 * 1024, 0x20000
    ~RingBuffer();
    size_t getNumUnread();
    unsigned int getReadPosition(); 
    unsigned int getWritePosition();
    unsigned long long getNumProcessed();
    uint8_t readNext();
    size_t populate(char* buff, size_t size, size_t exclude_from_front = 0);
    void setRollback();
    void doRollback();

    std::string extractString(size_t length);
  private:

    char* buffer_;
    const int size_;
    unsigned int read_position_;
    unsigned int write_position_;
    unsigned int rollback_;
    unsigned long long data_processed_;
    size_t unread_;
    size_t ring_mask_;
};

#endif