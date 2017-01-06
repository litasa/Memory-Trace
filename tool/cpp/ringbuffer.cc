#include "ringbuffer.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <stdlib.h>

RingBuffer::RingBuffer(int value) : capacity_(value) {
    data_ = new char[capacity_];
    //for dev should be removed
    memset(data_,0,capacity_);

    rollback_ = 0;
    read_pos_ = 0;
    write_pos_ = 0;
    unread_ = 0;
    num_read_ = 0;
}

RingBuffer::~RingBuffer() {
    free(data_);
    data_ = nullptr;
}

size_t RingBuffer::readNext(uint8_t& ret) {
    if(unread_ == 0) {
        return 0;
    }
    ret = (uint8_t)data_[read_pos_];
    read_pos_++;
    num_read_++;
    unread_--;
    if(read_pos_ == capacity_) {
        read_pos_ = 0;
    }
    return 1;
}

size_t RingBuffer::getNumUnread() {
    return unread_;
}

size_t RingBuffer::getReadPosition() {
    return read_pos_;
}

size_t RingBuffer::getWritePosition() {
    return write_pos_;
}

size_t RingBuffer::getNumProcessed() {
    return num_read_;
}

size_t RingBuffer::extractString(std::string& str, size_t length) {
    if(length == 0) {
        return 0;
    }
    size_t bytes_to_read = std::min(length, unread_);
    if(bytes_to_read <= capacity_ - read_pos_) {
        str.assign(data_ + read_pos_, bytes_to_read);
        read_pos_ += bytes_to_read;
        
        if(read_pos_ == capacity_) {
            read_pos_ = 0;
        }
    }
    else {
        size_t size_1 = capacity_ - read_pos_;
        str.assign(data_ + read_pos_, size_1);
        size_t size_2 = bytes_to_read - size_1;
        str.append(data_, size_2);
        read_pos_ = size_2;
    }
    unread_ -= bytes_to_read;
    num_read_ += bytes_to_read;
    return bytes_to_read;
}

size_t RingBuffer::populate(char* buff, size_t size) {
    if( size == 0) {
        return 0;
    }
    size_t bytes_to_write = std::min(size, capacity_ - unread_);
    if(bytes_to_write <= capacity_ - write_pos_) {
        memcpy(data_ + write_pos_, buff, bytes_to_write);
        write_pos_ += bytes_to_write;
        if(write_pos_ == capacity_) {
            write_pos_ = 0;
        }
    }
    else {
        size_t size_1 = capacity_ - write_pos_;
        memcpy(data_ + write_pos_, buff, size_1);
        size_t size_2 = bytes_to_write - size_1;
        memcpy(data_, buff + size_1, size_2);
        write_pos_ = size_2;
    }
    unread_ += bytes_to_write;
    return bytes_to_write;
}

void RingBuffer::setRollback() {
    rollback_ = read_pos_;
}

void RingBuffer::doRollback() {
    size_t read = read_pos_;
    if(rollback_ > read) {
      //if read_pos_ wrapped around
      read += capacity_;
  }
  size_t items_undone = read - rollback_;
  unread_ += items_undone;
  num_read_ -= items_undone;
  read_pos_ = rollback_;
}

void RingBuffer::printStats() {
    std::cout << "Printing Ringbuffer stats: ";
    std::cout << "\n\tRead pos: " << read_pos_;
    std::cout << "\n\tWrite pos: " << write_pos_;
    std::cout << "\n\tNum read: " << num_read_;
    std::cout << "\n\tRollback: " << rollback_;
    std::cout << "\n\tUnread: " << unread_;
    std::cout << "\n";
}
