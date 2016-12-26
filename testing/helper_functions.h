#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H
namespace helper {
    uint64_t decode(char* buffer) {
        uint64_t mul = 1;
        uint64_t val = 0;
        int pos = 0;
        uint8_t b = 0;
        do {
            b = (uint8_t)buffer[pos];
            
            val = val | (b*mul);
            mul = mul << 7;
            pos++;
        } while(b < 0x80);
        val &= ~mul;
        return val;
    }

    char* encode(uint64_t val, size_t& retsize) {
        char* out  = new char(10);
        uint8_t byte = 0;
        size_t i = 0;

        do
        {
            byte = (uint8_t) (val & 0x7f);
            out[i++] = byte;
            val = val >> 7;
        } while (val);
        out[i-1] = byte | 0x80;
        retsize = i;
        return out;
    }
} //namespace helper

#endif