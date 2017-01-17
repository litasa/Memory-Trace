#include "decoder.h"

#include <iostream>

#include <stdlib.h>
#include <math.h> //signbit

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

    //std::cout << "Starting new UnpackStream \n"; 

    size_t count = 0;
    size_t num_populated = 0;
    size_t total_populated = 0;
    RingBuffer* ring = obj->getRingbuffer();
    
    num_populated = ring->populate(buff, size);
    total_populated += num_populated;
    
    obj->trySteps();
    
    do {
      //std::cout << "=============================\nstarting main loop " << total_populated << " of " << size << " populated" <<" \n";
        do {
          ring->saveRollback();
          if(!obj->oneStep()) {
            //ring->doRollback();
            break;
          }
          ring->saveOverRollback();
          if(count > ring->getCapacity()) {
            //std::cout << "something is wrong in read: " << std::endl;
          }
          count++;
        }while(ring->getNumUnread());
        ring->loadRollback();
        //std::cout << "number of oneSteps before break: " << count << std::endl;
        num_populated = ring->populate(buff + total_populated, size - total_populated);
        total_populated += num_populated;
        //std::cout << "ending main loop\n=============================\n";
    }while(total_populated < size);

    //std::cout << "Ending UnpackStream\n";
    //std::cout << "last registerd eventnumber was: " << obj->registerd_events << "\n";
    ring->clearRollback();
    //ring->printStats();
}

NAN_METHOD(Decoder::Printas) {
  Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
}

NAN_METHOD(Decoder::GetMemoryAsArray) {
  Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
  unsigned int exclude_time = info[0]->Uint32Value();

  std::vector<Heap*> heaps = obj->getMemoryState();
 
  v8::Local<v8::Array> list_of_heaps = Nan::New<v8::Array>((int)heaps.size());

  double last_trend = 0;
  double previous_size = 0;
  for(int j = 0; j < heaps.size(); ++j) {
    std::vector<std::pair<size_t,size_t>>* allocs = &heaps[j]->simple_allocation_events_;
    v8::Local<v8::Array> allocation_list = Nan::New<v8::Array>((int)allocs->size());
    for(int i = 0; i < allocs->size(); ++i) {
      
      double time = (double)(*allocs)[i].first * obj->memory_state_->frequency_; // s
      double used_size = (double)((*allocs)[i].second);

      double trend = used_size - previous_size;
      if(i % 7 == 0) {
          v8::Local<v8::Object> object = Nan::New<v8::Object>();
          Nan::Set(object, Nan::New<v8::String>("x").ToLocalChecked(), Nan::New<v8::Number>(time)); //time
          Nan::Set(object, Nan::New<v8::String>("y").ToLocalChecked(), Nan::New<v8::Number>(used_size)); //allocation
          Nan::Set(allocation_list, i, object);
      }
      last_trend = trend;
      previous_size = used_size;
    }
    allocs->clear();
    v8::Local<v8::Object> heap_obj = Nan::New<v8::Object>();
    Nan::Set(heap_obj, Nan::New<v8::String>("label").ToLocalChecked(), Nan::New<v8::String>(heaps[j]->getName().c_str()).ToLocalChecked());
    Nan::Set(heap_obj, Nan::New<v8::String>("data").ToLocalChecked(), allocation_list);
    Nan::Set(list_of_heaps, j, heap_obj);
  }
  info.GetReturnValue().Set(list_of_heaps);
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