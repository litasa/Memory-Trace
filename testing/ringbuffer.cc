#include "ringbuffer.h"

#include <iostream>


RingBuffer::RingBuffer(int value) : size_(value) {
    if(value % 2 != 0) {
        std::cout << "need to be divisible by 2 to work properly" << std::endl;
    }
    buffer_ = new char[size_];
    //for dev should be removed
    memset(buffer_,0,size_);

    ring_mask_ = size_ - 1; //128 * 1024 - 1, 0x1ffff
    rollback_ = 0;
    read_position_ = 0;
    unread_ = 0;
}

RingBuffer::~RingBuffer() {
    free(buffer_);
    buffer_ = nullptr;
}

uint8_t RingBuffer::getByte() {
    read_position_ = ring_mask_ & read_position_;
    uint8_t ret = (uint8_t)buffer_[read_position_];
    read_position_++;
    unread_--;
    return ret;
}

size_t RingBuffer::getWritePos() {
    return (read_position_ + unread_) & ring_mask_;
}

size_t RingBuffer::populate(char* buffer, size_t size) {
  size_t space_left = size_ - unread_;
  size_t write_pos = getWritePos();
  //determine number of items to copy
  size_t num_to_copy = min(size, space_left);
  if(num_to_copy > 0) {
    memcpy(buffer_ + write_pos, buffer, num_to_copy);
    unread_ += num_to_copy;
  }
  return size - num_to_copy;
}

void RingBuffer::setRollback() {
    rollback_ = read_position_;
}

void RingBuffer::doRollback() {
    if(rollback_ > read_position_) {
      //if read_position_ wrapped around
      read_position_ += size_;
  }
  size_t items_undone = read_position_ - rollback_;
  unread_ += items_undone;
  read_position_ = rollback_;
}