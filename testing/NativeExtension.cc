#include "functions.h"

using v8::FunctionTemplate;

// NativeExtension.cc represents the top level of the module.
// C++ constructs that are exposed to javascript are exported here

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New("unpackBuffer").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(unpackBuffer)).ToLocalChecked());

    Nan::Set(target, Nan::New("encodeValue").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(encodeValue)).ToLocalChecked());
}

NODE_MODULE(NativeExtension, InitAll)
