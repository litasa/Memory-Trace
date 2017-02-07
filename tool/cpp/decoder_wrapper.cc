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
  Nan::SetPrototypeMethod(tpl, "getCurrentMemory", GetCurrentMemoryUsage);
  
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
    size_t total_populated = 0;
    RingBuffer* ring = obj->getRingbuffer();
    
    num_populated = ring->populate(buff, size);
    total_populated += num_populated;
    obj->trySteps();
    Event::Event* event;
    do {
        do {
          ring->saveRollback();
          event = obj->oneStep();
          if(event == nullptr) {
            break;
          }
          ring->saveOverRollback();
          count++;
          obj->memory_state_->addEvent(event);
        }while(ring->getNumUnread());
        ring->loadRollback();
        num_populated = ring->populate(buff + total_populated, size - total_populated);
        total_populated += num_populated;
    }while(total_populated < size);

    ring->clearRollback();
}

NAN_METHOD(Decoder::Printas) {
  Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
}

NAN_METHOD(Decoder::GetMemoryAsArray) {
    Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());

  std::vector<Heap*> heaps = obj->getMemoryState();
 
  v8::Local<v8::Array> list_of_heaps = Nan::New<v8::Array>((int)heaps.size());

  int removed = 0;
  // /* ========= Moving Average filtering ========= */
  for(int j = 0; j < heaps.size(); ++j) {
    std::vector<std::pair<size_t,size_t>>* allocs = &heaps[j]->simple_allocation_events_;
    if(allocs->size() == 0) {
      continue;
    }
    std::vector<v8::Local<v8::Object>> alloc_list;
    int ma_sample_size = min(allocs->size(),800);
    for(int current_sample = 0; current_sample < allocs->size(); current_sample += ma_sample_size) {
        size_t sum_mem = 0;
        size_t sum_pos = 0;
        int samples = min(allocs->size() - current_sample, ma_sample_size);
        for(int k = current_sample; k < current_sample + samples; ++k) {
          sum_mem += (*allocs)[k].second;
          sum_pos += (*allocs)[k].first;
        }
        double time = (double)(sum_pos)/(double)(samples);
        time /= obj->memory_state_->frequency_;
        double used_size = (double)(sum_mem)/(double)(samples);
        v8::Local<v8::Object> object = Nan::New<v8::Object>();
        Nan::Set(object, Nan::New<v8::String>("x").ToLocalChecked(), Nan::New<v8::Number>(time)); //time
        Nan::Set(object, Nan::New<v8::String>("y").ToLocalChecked(), Nan::New<v8::Number>(used_size)); //allocation
        alloc_list.push_back(object);        
  }
  allocs->clear();  

  // /* ========= Filter on timesteps distance ========= */
  // for(int j = 0; j < heaps.size(); ++j) {
  //   std::vector<std::pair<size_t,size_t>>* allocs = &heaps[j]->simple_allocation_events_;
  //   std::vector<v8::Local<v8::Object>> alloc_list;
  //   for(int i = 0; i < allocs->size(); ++i) {
  //     double time = (double)(*allocs)[i].first; //* obj->memory_state_->frequency_;
  //     double used_size = (double)((*allocs)[i].second);
  //     double timedistance = 0;
  //     if(i != 0) {
  //       timedistance = time - (double)(*allocs)[i-1].first;
  //     }

  //     if(timedistance > 50) {
  //         v8::Local<v8::Object> object = Nan::New<v8::Object>();
  //         Nan::Set(object, Nan::New<v8::String>("x").ToLocalChecked(), Nan::New<v8::Number>(time)); //time
  //         Nan::Set(object, Nan::New<v8::String>("y").ToLocalChecked(), Nan::New<v8::Number>(used_size)); //allocation
  //         //std::cout << "x: " << time << " y: " << used_size << "\n";
  //         alloc_list.push_back(object);
  //     }
  //   }

    v8::Local<v8::Array> allocation_list = Nan::New<v8::Array>(alloc_list.size());
    for(int i = 0; i < alloc_list.size(); ++i) {
      Nan::Set(allocation_list, i, alloc_list[i]);
    }    

    v8::Local<v8::Object> heap_obj = Nan::New<v8::Object>();
    std::stringstream label;
    label << "(" << heaps[j]->id_ << ")" << " " << heaps[j]->getName();
    Nan::Set(heap_obj, Nan::New<v8::String>("label").ToLocalChecked(), Nan::New<v8::String>(label.str().c_str()).ToLocalChecked());
    Nan::Set(heap_obj, Nan::New<v8::String>("data").ToLocalChecked(), allocation_list);
    Nan::Set(heap_obj, Nan::New<v8::String>("type").ToLocalChecked(), Nan::New<v8::String>(heaps[j]->getType().c_str()).ToLocalChecked());
    Nan::Set(list_of_heaps, j, heap_obj);
  }
  info.GetReturnValue().Set(list_of_heaps);
  
}

NAN_METHOD(Decoder::GetCurrentMemoryUsage) {
  Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
  std::vector<Heap> dead_heaps = obj->memory_state_->dead_heaps;
  v8::Local<v8::Array> list_of_dead_heaps = Nan::New<v8::Array>((int)dead_heaps.size());

  for(size_t i = 0; i < dead_heaps.size(); ++i) {
    std::stringstream label;
    label << "(" << dead_heaps[i].id_ << ")" << " " << dead_heaps[i].getName();
    Nan::Set(list_of_dead_heaps,i,Nan::New<v8::String>(label.str().c_str()).ToLocalChecked());
  }
  obj->memory_state_->dead_heaps.clear();
  info.GetReturnValue().Set(list_of_dead_heaps);


  // std::cout << "Printing memory list:";
  // for(int i = 0; i < heaps.size(); ++i) {
  //         v8::Local<v8::Object> object = Nan::New<v8::Object>();
  //         double time = heaps[i]->getLastUpdate() * obj->memory_state_->frequency_;
  //         int current_size = heaps[i]->used_memory_;
  //         std::cout << "\n\tId: " << heaps[i]->id_ << " Name: " <<heaps[i]->getName() << " Managed size: " << heaps[i]->managed_memory_ << " Used Memory: ";
  //         std::cout << heaps[i]->used_memory_ << " type: " << heaps[i]->type_ << " backing:";
  //         heaps[i]->printBacking();
  //         std::cout << "\n";
  //         //Nan::Set(object, Nan::New<v8::String>("x").ToLocalChecked(), Nan::New<v8::Number>(time)); //time
  //         //Nan::Set(object, Nan::New<v8::String>("y").ToLocalChecked(), Nan::New<v8::Number>(current_size)); //current size
  //         //Nan::Set(list_of_heaps, i, object);
  // }
  // std::cout << "\n";
  //info.GetReturnValue().Set(list_of_heaps);
}