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
  Nan::SetPrototypeMethod(tpl, "getMemoryAsArray", GetMemoryAsArray);
  Nan::SetPrototypeMethod(tpl, "getFilteredMemorySnapshots", GetFilteredData);
  Nan::SetPrototypeMethod(tpl, "streamEnd", StreamEnd);

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

NAN_METHOD(Decoder::GetMemoryAsArray) {
  // Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());

  // std::vector<Heap*> heaps = obj->getMemoryState();
 
  // v8::Local<v8::Array> list_of_heaps = Nan::New<v8::Array>((int)heaps.size());

  // // /* ========= Moving Average filtering ========= */
  // for(int j = 0; j < heaps.size(); ++j) {
  //   std::vector<std::pair<size_t,size_t>>* allocs = &heaps[j]->simple_allocation_events_;
  //   if(allocs->size() == 0) {
  //     continue;
  //   }
  //   std::vector<v8::Local<v8::Object>> alloc_list;
  //   size_t ma_sample_size = min(allocs->size(),800);
  //   for(size_t current_sample = 0; current_sample < allocs->size(); current_sample += ma_sample_size) {
  //       size_t sum_mem = 0;
  //       size_t sum_pos = 0;
  //       size_t samples = min(allocs->size() - current_sample, ma_sample_size);
  //       for(size_t k = current_sample; k < current_sample + samples; ++k) {
  //         sum_mem += (*allocs)[k].second;
  //         sum_pos += (*allocs)[k].first;
  //       }
  //       double time = (double)(sum_pos)/(double)(samples);
  //       time /= obj->memory_state_->frequency_;
  //       double used_size = (double)(sum_mem)/(double)(samples);
  //       v8::Local<v8::Object> object = Nan::New<v8::Object>();
  //       Nan::Set(object, Nan::New<v8::String>("x").ToLocalChecked(), Nan::New<v8::Number>(time)); //time
  //       Nan::Set(object, Nan::New<v8::String>("y").ToLocalChecked(), Nan::New<v8::Number>(used_size)); //allocation
  //       alloc_list.push_back(object);        
  // }
  // allocs->clear();  

  // // /* ========= Filter on timesteps distance ========= */
  // // for(int j = 0; j < heaps.size(); ++j) {
  // //   std::vector<std::pair<size_t,size_t>>* allocs = &heaps[j]->simple_allocation_events_;
  // //   std::vector<v8::Local<v8::Object>> alloc_list;
  // //   for(int i = 0; i < allocs->size(); ++i) {
  // //     double time = (double)(*allocs)[i].first; //* obj->memory_state_->frequency_;
  // //     double used_size = (double)((*allocs)[i].second);
  // //     double timedistance = 0;
  // //     if(i != 0) {
  // //       timedistance = time - (double)(*allocs)[i-1].first;
  // //     }

  // //     if(timedistance > 50) {
  // //         v8::Local<v8::Object> object = Nan::New<v8::Object>();
  // //         Nan::Set(object, Nan::New<v8::String>("x").ToLocalChecked(), Nan::New<v8::Number>(time)); //time
  // //         Nan::Set(object, Nan::New<v8::String>("y").ToLocalChecked(), Nan::New<v8::Number>(used_size)); //allocation
  // //         //std::cout << "x: " << time << " y: " << used_size << "\n";
  // //         alloc_list.push_back(object);
  // //     }
  // //   }

  //   v8::Local<v8::Array> allocation_list = Nan::New<v8::Array>((uint32_t)alloc_list.size());
  //   for(int i = 0; i < alloc_list.size(); ++i) {
  //     Nan::Set(allocation_list, i, alloc_list[i]);
  //   }    

  //   v8::Local<v8::Object> heap_obj = Nan::New<v8::Object>();
  //   std::stringstream label;
  //   label << "(" << heaps[j]->id_ << ")" << " " << heaps[j]->getName();
  //   Nan::Set(heap_obj, Nan::New<v8::String>("label").ToLocalChecked(), Nan::New<v8::String>(label.str().c_str()).ToLocalChecked());
  //   Nan::Set(heap_obj, Nan::New<v8::String>("data").ToLocalChecked(), allocation_list);
  //   Nan::Set(heap_obj, Nan::New<v8::String>("type").ToLocalChecked(), Nan::New<v8::String>(heaps[j]->getType().c_str()).ToLocalChecked());
  //   Nan::Set(list_of_heaps, j, heap_obj);
  // }
  // info.GetReturnValue().Set(list_of_heaps);
}

NAN_METHOD(Decoder::GetFilteredData) {
  Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
  size_t window_size = info[0]->IntegerValue();
  size_t from_sec = info[1]->IntegerValue();
  size_t to_sec = from_sec + window_size;
  size_t heap_number = info[2]->IntegerValue();
  size_t max_samples_per_second = info[3]->IntegerValue();
  size_t byte_conversion = info[4]->IntegerValue();

  //convert time to cpu ticks
  to_sec = to_sec*obj->memory_state_->frequency_;
  from_sec *= obj->memory_state_->frequency_;
  size_t skip_rate = (1/(double)max_samples_per_second)*obj->memory_state_->frequency_;

  Heap* heap = obj->memory_state_->getHeap(heap_number);
  if(heap == nullptr) {
    info.GetReturnValue().Set(Nan::False());
    return;
  }
  auto fromIter = heap->simple_allocation_events_.lower_bound(from_sec);
  auto toIter = heap->simple_allocation_events_.upper_bound(to_sec);
  if(toIter == heap->simple_allocation_events_.end()) {
    toIter--;
  }
  std::vector<std::pair<size_t,heap_usage*>> temp;
  heap_usage* lastMemory = &(fromIter->second);
  while(fromIter != heap->simple_allocation_events_.end()) {
    if(fromIter->first - from_sec > skip_rate) {
      temp.push_back(std::make_pair(from_sec, lastMemory));
    }
    else {
      lastMemory = &(fromIter->second);
      temp.push_back(std::make_pair(fromIter->first, lastMemory));
    }
    from_sec += skip_rate;
    fromIter = heap->simple_allocation_events_.lower_bound(from_sec);
  }

  v8::Local<v8::Array> allocation_data = Nan::New<v8::Array>((int)temp.size());
  v8::Local<v8::Array> managed_data = Nan::New<v8::Array>((int)temp.size());

  double time = 0;
  for(size_t i = 0; i < temp.size(); ++i) {
    double time = temp[i].first/obj->memory_state_->frequency_;
    size_t used_memory = temp[i].second->used_memory;
    size_t managed_memory = temp[i].second->managed_memory;

    for(int j = 0; j < byte_conversion; ++j) {
      used_memory >>= 10;
      managed_memory >>= 10;
    }

    v8::Local<v8::Object> used_scatter = Nan::New<v8::Object>();
    Nan::Set(used_scatter, Nan::New<v8::String>("x").ToLocalChecked(), Nan::New<v8::Number>(time)); //time
    Nan::Set(used_scatter, Nan::New<v8::String>("y").ToLocalChecked(), Nan::New<v8::Number>(used_memory));
    
    v8::Local<v8::Object> managed_scatter = Nan::New<v8::Object>();
    Nan::Set(managed_scatter, Nan::New<v8::String>("x").ToLocalChecked(), Nan::New<v8::Number>(time));
    Nan::Set(managed_scatter, Nan::New<v8::String>("y").ToLocalChecked(), Nan::New<v8::Number>(managed_memory));

    Nan::Set(allocation_data, i, used_scatter);
    Nan::Set(managed_data, i, managed_scatter);
  }

  v8::Local<v8::Object> used_dataset = Nan::New<v8::Object>();
  v8::Local<v8::Object> managed_dataset = Nan::New<v8::Object>();
  
  /*
  * Creates the JavaScript object: (content can change from the following example)
  {
    label: heap->getName,
    allocationType: heap->getAllocatorType,
    data: [{x: time, y: used_data}, {x: .., y: ..}, ..],
    spanGaps: true,
    borderWidth: 1,
    showLines: true,
    pointRadius: 1,
    lineTension: 0
  }
  */
  Nan::Set(used_dataset, Nan::New<v8::String>("label").ToLocalChecked(), 
                         Nan::New<v8::String>(heap->getName().c_str()).ToLocalChecked());
  Nan::Set(used_dataset, Nan::New<v8::String>("allocatorType").ToLocalChecked(), 
                         Nan::New<v8::String>(heap->getType().c_str()).ToLocalChecked());
  Nan::Set(used_dataset, Nan::New<v8::String>("data").ToLocalChecked(), allocation_data);
  
  Nan::Set(used_dataset, Nan::New<v8::String>("spanGaps").ToLocalChecked(), Nan::True());
  Nan::Set(used_dataset, Nan::New<v8::String>("borderWidth").ToLocalChecked(), Nan::New<v8::Number>(1));
  Nan::Set(used_dataset, Nan::New<v8::String>("showLines").ToLocalChecked(), Nan::True());
  Nan::Set(used_dataset, Nan::New<v8::String>("pointRadius").ToLocalChecked(), Nan::New<v8::Number>(1));
  Nan::Set(used_dataset, Nan::New<v8::String>("lineTension").ToLocalChecked(), Nan::New<v8::Number>(0));
  
  /*
  * Creates the JavaScript object: (content can change from the following example)
  {
    label: heap->getName,
    allocationType: heap->getAllocatorType,
    data: [{x: time, y: used_data}, {x: .., y: }, ..],
    spanGaps: true,
    borderWidth: 1,
    showLines: true,
    pointRadius: 1,
    lineTension: 0
  }
  */
  Nan::Set(managed_dataset, Nan::New<v8::String>("label").ToLocalChecked(), 
                         Nan::New<v8::String>(heap->getName().c_str()).ToLocalChecked());
  Nan::Set(managed_dataset, Nan::New<v8::String>("allocatorType").ToLocalChecked(), 
                         Nan::New<v8::String>(heap->getType().c_str()).ToLocalChecked());
  Nan::Set(managed_dataset, Nan::New<v8::String>("data").ToLocalChecked(), managed_data);
  
  Nan::Set(managed_dataset, Nan::New<v8::String>("spanGaps").ToLocalChecked(), Nan::True());
  Nan::Set(managed_dataset, Nan::New<v8::String>("borderWidth").ToLocalChecked(), Nan::New<v8::Number>(1));
  Nan::Set(managed_dataset, Nan::New<v8::String>("showLines").ToLocalChecked(), Nan::True());
  Nan::Set(managed_dataset, Nan::New<v8::String>("pointRadius").ToLocalChecked(), Nan::New<v8::Number>(1));
  Nan::Set(managed_dataset, Nan::New<v8::String>("lineTension").ToLocalChecked(), Nan::New<v8::Number>(0));

  v8::Local<v8::Array> chartData = Nan::New<v8::Array>(3);
  Nan::Set(chartData, 0, used_dataset);
  Nan::Set(chartData, 1, managed_dataset);
  Nan::Set(chartData, 2, Nan::New<v8::Number>(obj->memory_state_->last_update_ / obj->memory_state_->frequency_));
  info.GetReturnValue().Set(chartData);  
}

NAN_METHOD(Decoder::StreamEnd) {
  Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
  if(obj->_stream_end) {
    info.GetReturnValue().Set(Nan::True());
    return;
  }
  info.GetReturnValue().Set(Nan::False());
}