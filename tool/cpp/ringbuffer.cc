#include "ringbuffer.h"

#include <iostream>


RingBuffer::RingBuffer(int value) : capacity_(value) {
    data_ = new char[capacity_];
    //for dev should be removed
    memset(data_,0,capacity_);

    rollback_read_ = 0;
    read_pos_ = 0;

    rollback_write_ = 0;
    write_pos_ = 0;

    unread_ = 0;
    rollback_unread_ = 0;

    num_read_ = 0;
    rollback_num_read_ = 0;
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

size_t RingBuffer::getCapacity() {
    return capacity_;
}

size_t RingBuffer::extractString(std::string& str, size_t length) {
    if(length == 0) {
        return 0;
    }
    size_t bytes_to_read = min(length, unread_);
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
    size_t bytes_to_write = min(size, capacity_ - unread_);
    if(bytes_to_write <= capacity_ - write_pos_) {
        memcpy(data_ + write_pos_, buff, bytes_to_write);
        write_pos_ += bytes_to_write;
        if(write_pos_ == capacity_) {
            write_pos_ = 0;
        }
       //std::cout << "\ncopied: " << bytes_to_write << " using method 1: capacity_: "<< capacity_ << "\n";
        //std::cout << "read_pos_: " << read_pos_ << " write_pos_: " << write_pos_ << " unread_: " << unread_ <<" data at read_pos_" << (int)(uint8_t)data_[read_pos_] << "\n";
    }
    else {
        //std::cout << "\ncopied: " << bytes_to_write << " using method 2: capacity_: "<< capacity_ << "\n";
        //std::cout << "read_pos_: " << read_pos_ << " write_pos_: " << write_pos_ << " unread_: " << unread_ << "\n";
        size_t size_1 = capacity_ - write_pos_;
        memcpy(data_ + write_pos_, buff, size_1);
        //std::cout << "copied size_1: " << size_1 << " from write_pos_: " << " data at read_pos_" << (int)(uint8_t)data_[read_pos_] << "\n";
        size_t size_2 = bytes_to_write - size_1;
        memcpy(data_, buff + size_1, size_2);
        write_pos_ = size_2;
        //std::cout << "copied size_2: " << size_2 << " from 0" << " data at read_pos_" << (int)(uint8_t)data_[read_pos_] << "\n";
        if(bytes_to_write == capacity_) {
        //std::cout << "memcomp: " << memcmp(data_,buff,capacity_);
        }
    }
    unread_ += bytes_to_write;
    return bytes_to_write;
}

void RingBuffer::printStats() {
    std::cout << "Printing Ringbuffer stats: ";
    std::cout << "\n\tRead pos: " << read_pos_;
    std::cout << "\n\tWrite pos: " << write_pos_;
    std::cout << "\n\tNum read: " << num_read_;
    std::cout << "\n\tUnread: " << unread_;
    std::cout << "\n\tNumber of rollbacks " << rollbacks_.size();
    std::cout << "\n\n";
}

void RingBuffer::saveRollback() {
    rollback roll;
    roll.read_pos = read_pos_;
    roll.write_pos = write_pos_;
    roll.num_read = num_read_;
    roll.unread = unread_;
    rollbacks_.push(roll);
}
void RingBuffer::saveOverRollback() {
    rollback roll = rollbacks_.top();
    roll.read_pos = read_pos_;
    roll.write_pos = write_pos_;
    roll.num_read = num_read_;
    roll.unread = unread_;
}

void RingBuffer::loadRollback() {
    rollback roll = rollbacks_.top();
    rollbacks_.pop();
    read_pos_ = roll.read_pos;
    write_pos_ = roll.write_pos;
    num_read_ = roll.num_read;
    unread_ = roll.unread; 
}

void RingBuffer::removeRollback() {
    rollbacks_.pop();
}

void RingBuffer::clearRollback() {
    while(!rollbacks_.empty()) {
        rollbacks_.pop();
    }
}