#include "functions.h"
#include "helper_functions.h"

#include <iostream>

NAN_METHOD(decodeValue) {
    char* buffer = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);
	
    uint64_t val = helper::decode(buffer);

    //let v8 handle garbage collection
    char* retval = new char[size];
	for (int i = 0; i < size; ++i) {
		char var = val & 0xff;
		retval[i] = var;
		val = val >> 8;
	}
   
    info.GetReturnValue().Set(Nan::NewBuffer(retval, size).ToLocalChecked());
}

NAN_METHOD(encodeValue) {
    char* buffer = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);
    uint64_t value = 0;
    uint64_t mul = 1;
    for(int i = 0; i < size; ++i) {
       value  +=  (uint8_t)buffer[i] * mul;
       mul = mul << 8;
    }
	//let v8 handle garbage collection
    size_t retsize = 0;
    char* retval = helper::encode(value, retsize);
    info.GetReturnValue().Set(Nan::NewBuffer(retval, retsize).ToLocalChecked());
}