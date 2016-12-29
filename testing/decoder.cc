#include "decoder.h"
#include <iostream>
Decoder::Decoder() {
    ring_ = new RingBuffer();
}

RingBuffer* Decoder::getRingbuffer() {
    return ring_;
}

Decoder::~Decoder() {
    delete ring_;
}

bool Decoder::decodeValue(uint64_t& ret) {
    uint64_t mul = 1;
    ret = 0;
    uint8_t b = 0;
    do {
        if(ring_->getNumUnread() == 0) {
            ret = -1;
            return false;
        }
        b = ring_->readNext();
        ret = ret | (b*mul);
        mul = mul << 7;
    } while(b < 0x80);
    ret &= ~mul;
    return true;
}

bool Decoder::decodeString(std::string& ret) {
        uint64_t length;
        if(!decodeValue(length)) {
            std::cout << "Decode_string: length failed";
            return false;
        }
        if(length > ring_->getNumUnread()) {
            std::cout << "Decode_string: length(" << length << ") is longer then number of unread(" << ring_->getNumUnread() << ")\n";
            return false;
        }
        ret = ring_->extractString(length);
        return true;
}