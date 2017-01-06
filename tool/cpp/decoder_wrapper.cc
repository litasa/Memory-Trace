#include "decoder.h"

#include <iostream>

#include <ctime>

Nan::Persistent<v8::Function> Decoder::constructor;

NAN_MODULE_INIT(Decoder::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Decoder").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "unpackStream", UnpackStream);
  Nan::SetPrototypeMethod(tpl, "printas", Printas);
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
    v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
    info.GetReturnValue().Set(Nan::NewInstance(cons,argc, argv).ToLocalChecked());
  }
}

NAN_METHOD(Decoder::UnpackStream) {
    Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
    char* buff = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);

    size_t count = 0;
    size_t num_populated = 0;
    RingBuffer* ring = obj->getRingbuffer();

    std::clock_t start = std::clock();
    do {
        ring->doRollback();
        num_populated = ring->populate(buff + num_populated, size - num_populated);
        obj->oneStep();
        if(count > size) {
          std::cout << "we have problems "  << num_populated << " " << size<< std::endl;
          ring->printStats();
        }
        count++;
    }while(num_populated < size);
    std::clock_t end = std::clock();
    std::cout << "took: " << double(end-start)/CLOCKS_PER_SEC << " seconds" << std::endl;
    //obj->printMemoryState();
}

NAN_METHOD(Decoder::Printas) {
  Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
  obj->printMemoryState();
}