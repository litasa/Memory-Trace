#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <string>

namespace helper {

    uint64_t decode(char* buffer, size_t& pos) {
        uint64_t mul = 1;
        uint64_t val = 0;
        size_t i = pos;
        uint8_t b = 0;
        do {
            b = (uint8_t)buffer[i];
            
            val = val | (b*mul);
            mul = mul << 7;
            i++;
        } while(b < 0x80);
        val &= ~mul;
        pos = i;
        return val;
    }

    std::string decodeString(char* buffer, size_t& pos) {
        uint64_t length = decode(buffer,pos);
        std::string str((buffer + pos), length);
        pos += length;
        return str;
    }

    char* encode(uint64_t val, size_t& pos, size_t& encode_size) {
        char* out  = new char(10);
        uint8_t byte = 0;
        size_t i = pos;

        do
        {
            byte = (uint8_t) (val & 0x7f);
            out[i++] = byte;
            val = val >> 7;
        } while (val);
        out[i-1] = byte | 0x80;
        encode_size = i - pos;
        pos = i;
        return out;
    }
} //namespace helper

#endif