#include "functions.h"

using v8::FunctionTemplate;

// NativeExtension.cc represents the top level of the module.
// C++ constructs that are exposed to javascript are exported here

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New("decodeValue").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(DecodeValue)).ToLocalChecked());

    Nan::Set(target, Nan::New("decodeString").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(DecodeString)).ToLocalChecked());

    Nan::Set(target, Nan::New("encodeString").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(EncodeString)).ToLocalChecked());

    Nan::Set(target, Nan::New("encodeValue").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(EncodeValue)).ToLocalChecked());

    //for testing purposes
    Nan::Set(target, Nan::New("encodeNumber").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(EncodeNumber)).ToLocalChecked());

    Nan::Set(target, Nan::New("decodeStream").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(DecodeStream)).ToLocalChecked());

    RingBuffer::Init(target);
}

NODE_MODULE(NativeExtension, InitAll)
