#include "decoder.h"

#include <iostream>
#include <iomanip>
#include <fstream>

#include <ctime>

Nan::Persistent<v8::Function> Decoder::constructor;

NAN_MODULE_INIT(Decoder::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Decoder").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "unpackStream", UnpackStream);
  /*Debug - start*/
  /*Debug - end*/

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("Decoder").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(Decoder::New) {
  if (info.IsConstructCall()) {
    Decoder *obj = new Decoder();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 1; 
    v8::Local<v8::Value> argv[argc] = {info[0]};
    v8::Local<v8::Function> cons = Nan::New(constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}

NAN_METHOD(Decoder::UnpackStream) {
    Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
    char* buff = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);
    v8::String::Utf8Value val(info[1]->ToString());
    std::string file_name(*val);
    file_name.append(".log");
    RingBuffer* ring = obj->getRingbuffer();
    //std::ofstream outFile;
    //outFile.open(file_name);

    //outFile << std::showbase;
    //v8::Local<v8::Array> arr = Nan::New<v8::Array>((int)(size/2)); //maybe even divided by 4 could be fine
    size_t num_already_populated = 0;
    size_t count = 0;
    std::clock_t start = std::clock();
    do {
      count++;
        ring->doRollback();
        num_already_populated = ring->populate(buff, size, num_already_populated);
        obj->oneStep();
    }while(num_already_populated < size);
    std::clock_t end = std::clock();
    std::cout << "took: " << double(end-start)/CLOCKS_PER_SEC << " seconds" << std::endl;
}