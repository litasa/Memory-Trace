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
}

RingBuffer::~RingBuffer() {
    free(buffer_);
    buffer_ = nullptr;
}

uint8_t RingBuffer::readNext() {
    read_position_ = ring_mask_ & read_position_;
    uint8_t ret = (uint8_t)buffer_[read_position_];
    read_position_++;
    unread_--;
    return ret;
}

size_t RingBuffer::getNumUnread() {
    return unread_;
}

unsigned int RingBuffer::getReadPosition() {
    return read_position_;
}

std::string RingBuffer::extractString(size_t length) {
    
    size_t space_left = size_ - read_position_;
    size_t num_to_copy = min(length, space_left);
    std::cout << "length: " << length << " space_left: " << space_left << "num_to_copy: " << num_to_copy;
    std::string ret(buffer_ + read_position_, num_to_copy);
    
    read_position_ += num_to_copy;
    unread_ -= num_to_copy;
    std::cout << "first string copy: " << ret << ", read pos: " << read_position_ << " unread: " << unread_ <<std::endl;
    std::cout << "length: " << length << " num_to_copy: " << num_to_copy;
    size_t left_to_copy  = length - num_to_copy;
    if(left_to_copy > 0) {
        std::cout << "in extractString: " << left_to_copy << "> 0" << std::endl;
        ret.append(buffer_, left_to_copy);
        std::cout << "ret" << ret;
        read_position_ = left_to_copy; //this might give errors (setting read_position to one behind then what it is supposed to be)
        unread_ -= left_to_copy;
    }
    return ret;
}

size_t RingBuffer::populate(char* buff, size_t size, size_t num_already_copied) {
    std::cout << "populating: size = " << size << " already copied: " << num_already_copied;
    size_t num_to_copy;
    //size -= exclude_from_front;
    
    if(read_position_ <= write_position_) {
        size_t space_to_end = size_ - write_position_;
        num_to_copy = min(space_to_end, size);
        std::cout << "\n\twrite_position_ >= read_pos: " << write_position_ << " >= " << read_position_;
        std::cout << "\n\tspace_to_end: " << space_to_end;
        std::cout << "\n\tnum_to_copy: " << num_to_copy;
        std::cout << "\n\tnum unread before: " << unread_;
        memcpy(buffer_ + write_position_, buff + num_already_copied, num_to_copy);
        num_already_copied += num_to_copy;
        std::cout << "After populating the buffer contains: " << std::endl;
        for(int i = 1; i < size_ + 1; ++i) {
            std::cout << (int)((uint8_t)buffer_[i - 1]) << " ";
            if(i % 10 == 0) {
                std::cout << "\n";
            }
        }
        std::cout << std::endl;
        unread_ += num_to_copy;
        write_position_ = (write_position_ + num_to_copy) & ring_mask_;

        std::cout << "\n\twrite pos after: " << write_position_;
        std::cout << "\n\tnum unread after: " << unread_;
        std::cout << "\n\tnum already copied: " << num_already_copied;
        if(size == num_already_copied) {
            std::cout << "\n";
            return num_already_copied;
        }
        //so we still want to add stuff from buff. Do we have space left in the RingBuffer?
        size_t space_left = size_ - unread_;
        if(space_left) {
            std::cout << "\n\t\twe got space left: " << space_left;
            num_to_copy = min(read_position_, size - num_already_copied);
            memcpy(buffer_, buff + num_already_copied, num_to_copy);
            num_already_copied += num_to_copy;
            std::cout << "After populating the buffer contains WRAP AROUND: " << std::endl;
            for(int i = 1; i < size_ + 1; ++i) {
                std::cout << (int)((uint8_t)buffer_[i - 1]) << " ";
                if(i % 10 == 0) {
                    std::cout << "\n";
                }
            }
            write_position_ = (write_position_ + num_to_copy) & ring_mask_;
            unread_ += num_to_copy;
        }
        std::cout << "\n";
        return num_already_copied;
    }
    size_t space_to_read = read_position_;
    num_to_copy = min(read_position_, size);
    std::cout << "\n\twrite_position_ < read_pos: ";
    std::cout << "\n\tspace_to_read: " << space_to_read;
    std::cout << "\n\tnum_to_copy: " << num_to_copy;
    std::cout << "\n\twrite pos before: " << write_position_;
    std::cout << "\n\tread pos: " << read_position_;
    std::cout << "\n\tnum unread: " << unread_;
    memcpy(buffer_ + write_position_, buff + num_already_copied, num_to_copy);
    num_already_copied += num_to_copy;
    std::cout << "After populating the buffer contains: " << std::endl;
        for(int i = 1; i < size_ + 1; ++i) {
            std::cout << (int)((uint8_t)buffer_[i - 1]) << " ";
            if(i % 10 == 0) {
                std::cout << "\n";
            }
        }
    write_position_ = (write_position_ + num_to_copy) & ring_mask_;
    unread_ += num_to_copy;
    std::cout << "\n\twrite pos after: " << write_position_;
    std::cout << "\n";
    return num_already_copied;
}
    


  /*
  std::cout << "populating";
  size_t space_left = size_ - unread_;
  size_t write_pos = getWritePos();
  std::cout << "\n\tspace left: " << space_left;
  std::cout << "\n\twrite pos: " << write_pos;
  std::cout << "\n\tread pos: " << read_position_;
  //determine number of items to copy
  size_t num_to_copy = min(size, space_left);
  std::cout << "\n\tnumber to copy: " << num_to_copy;
  if(num_to_copy > 0) {
    memcpy(buffer_ + write_pos, buff, num_to_copy);
    unread_ += num_to_copy;
  }
  std::cout << "\n\tnum not populated: " << size - num_to_copy << std::endl;
  return size - num_to_copy;
  */

void RingBuffer::setRollback() {
    rollback_ = read_position_;
}

void RingBuffer::doRollback() {
  std::cout << "Rolling back: ";
  std::cout << "\n\tread_position before: " << read_position_;
    if(rollback_ > read_position_) {
      //if read_position_ wrapped around
      read_position_ += size_;
  }
  size_t items_undone = read_position_ - rollback_;
  unread_ += items_undone;
  read_position_ = rollback_;
  std::cout << "\n\tread_position after: " << rollback_ << "\n";
  std::cout << "\n\titems_unread" << items_undone << "\n";
}