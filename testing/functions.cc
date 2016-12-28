#include "functions.h"
#include "helper_functions.h"

#include <iostream>
#include <algorithm> //for min

NAN_METHOD(DecodeValue) {
    char* buffer = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);
	size_t readPos = 0;
    uint64_t val = helper::decode(buffer, readPos);

    //let v8 handle garbage collection
    char* retval = new char[size];
	for (int i = 0; i < size; ++i) {
		char var = val & 0xff;
		retval[i] = var;
		val = val >> 8;
	}
   
    info.GetReturnValue().Set(Nan::NewBuffer(retval, (uint32_t)size).ToLocalChecked());
}

NAN_METHOD(DecodeString) {
    char* buffer = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);
	size_t readPos = 0;
    std::string str = helper::decodeString(buffer, readPos);
   
    info.GetReturnValue().Set(Nan::New(str.c_str()).ToLocalChecked());
}

NAN_METHOD(EncodeString) {
    v8::String::Utf8Value val(info[0]->ToString());
    const size_t   len   = val.length();

    size_t val_length;
    size_t readPos = 0;
    char* enc_length = helper::encode(len, readPos, val_length);

    char* retval = new char[val_length + len];

    memcpy(retval, enc_length, val_length); //string length
    memcpy(retval + val_length, *val, len); //string content

    info.GetReturnValue().Set(Nan::NewBuffer(retval, (uint32_t)(val_length + len)).ToLocalChecked());
}

NAN_METHOD(EncodeValue) {
    char* buffer = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);
    uint64_t value = 0;
    uint64_t mul = 1;

    for(int i = 0; i < size; ++i) {
       value  +=  (uint8_t)buffer[i] * mul;
       mul = mul << 8;
    }

    size_t retsize = 0;
    size_t readPos = 0;
    char* retval = helper::encode(value, readPos, retsize);
    info.GetReturnValue().Set(Nan::NewBuffer(retval, (uint32_t)(retsize)).ToLocalChecked());
}

//for testing purposes
NAN_METHOD(EncodeNumber) {
    int value = Nan::To<int>(info[0]).FromJust(); 

    size_t retsize = 0;
    size_t readPos = 0;
    char* retval = helper::encode(value, readPos, retsize);

    int val = 0;
    int mul = 1;
    for(int i = 0; i < retsize; ++i) {
       val  +=  (uint8_t)retval[i] * mul;
       mul = mul << 8;
    }

    info.GetReturnValue().Set(val);
}

NAN_METHOD(DecodeStream) {
    char* buffer = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);
    size_t readPos = 0;
    info.GetReturnValue().Set((uint32_t)size);
}






// Wrapper Impl

