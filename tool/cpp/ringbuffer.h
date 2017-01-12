#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include <nan.h>
#include <string>
#include <stack>

//insired by http://asawicki.info/news_1468_circular_buffer_of_raw_binary_data_in_c.html
class RingBuffer {
  public:
    explicit RingBuffer(int capacity = 128*1024); //128 * 1024, 0x20000
    ~RingBuffer();
    size_t getNumUnread();
    size_t getReadPosition(); 
    size_t getWritePosition();
    size_t getNumProcessed();
    size_t getCapacity();
    size_t readNext(uint8_t& byte);
    size_t populate(char* buff, size_t size);

    size_t extractString(std::string& str, size_t length);

    void printStats();
    void saveRollback();
    void saveOverRollback();
    void loadRollback();
    void removeRollback();
    void clearRollback();
  private:

    struct rollback {
      size_t read_pos;
      size_t write_pos;
      size_t num_read;
      size_t unread;
    };
    std::stack<rollback> rollbacks_;

    char* data_;
    const size_t capacity_;
    size_t read_pos_;
    size_t write_pos_;
    size_t num_read_;
    size_t unread_;
    size_t rollback_read_;

    size_t rollback_write_;
    size_t rollback_unread_;

    size_t rollback_num_read_;
};

#endif