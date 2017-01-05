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
    write_position_ = 0;
    unread_ = 0;
    data_processed_ = 0;
}

RingBuffer::~RingBuffer() {
    free(buffer_);
    buffer_ = nullptr;
}

uint8_t RingBuffer::readNext() {
    read_position_ = ring_mask_ & read_position_;
    uint8_t ret = (uint8_t)buffer_[read_position_];
    read_position_++;
    data_processed_++;
    unread_--;
    return ret;
}

size_t RingBuffer::getNumUnread() {
    return unread_;
}

size_t RingBuffer::getReadPosition() {
    return read_position_;
}

size_t RingBuffer::getWritePosition() {
    return write_position_;
}

size_t RingBuffer::getNumProcessed() {
    return data_processed_;
}

std::string RingBuffer::extractString(size_t length) {
    
    size_t space_left = size_ - read_position_;
    size_t num_to_copy = min(length, space_left);
    std::string ret(buffer_ + read_position_, num_to_copy);
    
    read_position_ += num_to_copy;
    unread_ -= num_to_copy;
    size_t left_to_copy  = length - num_to_copy;
    if(left_to_copy > 0) {
        ret.append(buffer_, left_to_copy);
        read_position_ = left_to_copy; //this might give errors (setting read_position to one behind then what it is supposed to be)
        unread_ -= left_to_copy;
    }
    return ret;
}

size_t RingBuffer::populate(char* buff, size_t size, size_t num_already_copied) {
    size_t num_to_copy;

    if(read_position_ <= write_position_) {
        size_t space_to_end = size_ - write_position_;
        num_to_copy = min(space_to_end, size);
        num_to_copy = min(size - num_already_copied, num_to_copy);
        memcpy(buffer_ + write_position_, buff + num_already_copied, num_to_copy);
        num_already_copied += num_to_copy;

        unread_ += num_to_copy;
        write_position_ = (write_position_ + num_to_copy) & ring_mask_;


        if(size == num_already_copied) {
            return num_already_copied;
        }
        //so we still want to add stuff from buff. Do we have space left in the RingBuffer?
        size_t space_left = size_ - unread_;
        if(space_left) {
            num_to_copy = min(read_position_ - 1, size - num_already_copied);
            memcpy(buffer_, buff + num_already_copied, num_to_copy);
            num_already_copied += num_to_copy;

            write_position_ = (write_position_ + num_to_copy) & ring_mask_;
            unread_ += num_to_copy;
        }

        return num_already_copied;
    }

    num_to_copy = read_position_ - write_position_;
    memcpy(buffer_ + write_position_, buff + num_already_copied, num_to_copy);
    num_already_copied += num_to_copy;
    write_position_ = (write_position_ + num_to_copy) & ring_mask_;
    unread_ += num_to_copy;
    return num_already_copied;
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