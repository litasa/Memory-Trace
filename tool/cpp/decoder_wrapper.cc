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
  Nan::SetPrototypeMethod(tpl, "getMemoryAsArray", GetMemoryAsArray);
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
        do {
          ring->setRollback();
          if(!obj->oneStep()) {
            break;
          }
          ring->setRollback();
        }while(ring->getNumUnread());
        ring->doRollback();
        if(count > size) {
          std::cout << "we have problems "  << num_populated << " " << size<< std::endl;
          ring->printStats();
          throw;
        }
        count++;
    }while(num_populated < size);
    std::clock_t end = std::clock();
    std::cout << "took: " << double(end-start)/CLOCKS_PER_SEC << " seconds" << std::endl;
}

NAN_METHOD(Decoder::Printas) {
  Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
}

NAN_METHOD(Decoder::GetMemoryAsArray) {
  Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
  auto ret = obj->getMemoryState();
  v8::Local<v8::Array> result_list = Nan::New<v8::Array>((int)ret.size());
  for(unsigned int i = 0; i < ret.size(); ++i) {
    v8::Local<v8::Object> obj = Nan::New<v8::Object>();
    if(ret[i] == nullptr) {
      std::cout << "hmm error null ptr" << std::endl;
    }
    Nan::Set(obj, Nan::New("name").ToLocalChecked(), Nan::New(ret[i]->getName().c_str()).ToLocalChecked());
    Nan::Set(obj, Nan::New("used_memory").ToLocalChecked(), Nan::New((int)ret[i]->getUsedMemory()));
    Nan::Set(obj, Nan::New("last_update").ToLocalChecked(), Nan::New((int)ret[i]->getLastUpdate()));
    Nan::Set(result_list, i, obj);
  }
  info.GetReturnValue().Set(result_list);
}

NAN_METHOD(Decoder::GetNewEvents) {
  // Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
  // //auto ret = obj->getNewEvents();
  // v8::Local<v8::Array> result_list = Nan::New<v8::Array>((int)ret.size());
  // for(unsigned int i = 0; i < ret.size(); ++i) {
  //   v8::Local<v8::Object> obj = Nan::New<v8::Object>();
  //   if(ret[i] == nullptr) {
  //     std::cout << "hmm error null ptr" << std::endl;
  //   }
  //   Nan::Set(obj, Nan::New("name").ToLocalChecked(), Nan::New(ret[i]->getName().c_str()).ToLocalChecked());
  //   Nan::Set(obj, Nan::New("used_memory").ToLocalChecked(), Nan::New((int)ret[i]->getUsedMemory()));
  //   Nan::Set(obj, Nan::New("last_update").ToLocalChecked(), Nan::New((int)ret[i]->getLastUpdate()));
  //   Nan::Set(result_list, i, obj);
  // }
  // info.GetReturnValue().Set(result_list);
}