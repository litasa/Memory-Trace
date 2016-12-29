#include "decoder.h"

#include <iostream>
#include <iomanip>

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
    enum EventCode
    {
      BeginStream     = 1,
      EndStream,

      HeapCreate = 18,
      HeapDestroy,

      HeapAddCore,
      HeapRemoveCore,

      HeapAllocate,
      HeapFree,
    };
    Decoder* obj = Nan::ObjectWrap::Unwrap<Decoder>(info.This());
    char* buff = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);
    RingBuffer* ring = obj->getRingbuffer();

    v8::Local<v8::Array> arr = Nan::New<v8::Array>((int)(size/2)); //maybe even divided by 4 could be fine
    size_t num_not_populated = 0;
    do {
        num_not_populated = ring->populate(buff,size);

        do {
          v8::Local<v8::Object> js_obj = Nan::New<v8::Object>();
          size_t current_code;
          if(!obj->decodeValue(current_code)) {
              return;
          }

          size_t time_stamp;
          if(!obj->decodeValue(time_stamp)) {
              return;
          }
          std::cout << std::showbase;
          std::cout << "Event with code: " << current_code << " time: " << time_stamp << "\n";
          switch(current_code) {
            case BeginStream :
            {
              std::string platform;
              size_t system_frequency;
              size_t stream_magic;
              if(!obj->decodeValue(stream_magic)) {
                std::cout << "Decode stream_magic failed" << std::endl;
                return;
              }
              if(!obj->decodeString(platform)) {
                std::cout << "Decode platform failed" << std::endl;
                return;
              }
              if(!obj->decodeValue(system_frequency)) {
                std::cout << "Decode system_frequency failed" << std::endl;
                return;
              }
              std::cout << "BeginStream\nplatform: " << platform << "\nsystem frequency: " << system_frequency;
              break;
            }

            case EndStream :
              {
                std::cout << "Endstream: Do nothing";
              }
            break;

            case HeapCreate :
            { 
              std::string name;
              size_t id;
              if(!obj->decodeValue(id)) {return ;}
              if(!obj->decodeString(name)) { return ;}
              std::cout << "HeapCreate\nId: " << id << "\nName: " << name;
              break;
            }

            case HeapDestroy :
            {
              size_t id;
              if(!obj->decodeValue(id)) { return ; }
              std::cout << "HeapDestroy\nId: " << id;
              break;
            }

            case HeapAddCore :
            {
              size_t id;
              size_t pointer;
              size_t size_bytes;
              if(!obj->decodeValue(id)) {return ; }
              if(!obj->decodeValue(pointer)) { return ; }
              if(!obj->decodeValue(size_bytes)) { return ;}
              std::cout << "HeapAddCore\nId: " << id <<"\nPointer: " << std::hex << pointer << std::dec << "\nSize: " << size_bytes; 
              break;
            }

            case HeapRemoveCore :
            {  
              size_t id;
              size_t pointer;
              size_t size_bytes;
              if(!obj->decodeValue(id)) {return ; }
              if(!obj->decodeValue(pointer)) { return ; }
              if(!obj->decodeValue(size_bytes)) { return ;}
              std::cout << "HeapRemoveCore\nId: " << id <<"\nPointer: " << std::hex << pointer << std::dec << "\nSize: " << size_bytes; 
              break;
            }
            case HeapAllocate:
            {
              size_t id;
              size_t pointer;
              size_t size_bytes;
              if(!obj->decodeValue(id)) {return ; }
              if(!obj->decodeValue(pointer)) { return ; }
              if(!obj->decodeValue(size_bytes)) { return ;}
              std::cout << "HeapAllocate\nId: " << id <<"\nPointer: " << std::hex << pointer << std::dec << "\nSize: " << size_bytes; 
              break;
            }

            case HeapFree:
            { 
              size_t id;
              size_t pointer;
              if(!obj->decodeValue(id)) {return ; }
              if(!obj->decodeValue(pointer)) { return ; }
              std::cout << "HeapFree\nId: " << id <<"\nPointer: " << std::hex << pointer << std::dec; 
              break;
            }
            default:
              std::cout << "Unhandled Event";
            break;
          }
          std::cout << "\n\n";
        }while(ring->getNumUnread());
        
    }while(num_not_populated);

    std::cout << std::endl;
}