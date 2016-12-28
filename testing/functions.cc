#include "functions.h"
#include "helper_functions.h"

#include <iostream>
#include <algorithm>

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
   
    info.GetReturnValue().Set(Nan::NewBuffer(retval, size).ToLocalChecked());
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

    info.GetReturnValue().Set(Nan::NewBuffer(retval, val_length + len).ToLocalChecked());
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
    info.GetReturnValue().Set(Nan::NewBuffer(retval, retsize).ToLocalChecked());
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
    info.GetReturnValue().Set((int)size);
}






// Wrapper Impl

Nan::Persistent<v8::Function> RingBuffer::constructor;

NAN_MODULE_INIT(RingBuffer::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("RingBuffer").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "populate", Populate);
  Nan::SetPrototypeMethod(tpl, "readByte", ReadByte);
  Nan::SetPrototypeMethod(tpl, "setRollback", SetRollback);
  Nan::SetPrototypeMethod(tpl, "doRollback", DoRollback);

  /*Debug - start*/
  Nan::SetPrototypeMethod(tpl, "getBuffer", GetBuffer);
  Nan::SetPrototypeMethod(tpl, "getReadPos", GetReadPosition);
  Nan::SetPrototypeMethod(tpl, "getUnread", GetUnread);
  Nan::SetPrototypeMethod(tpl, "getWritePos", GetWritePosition);
  Nan::SetPrototypeMethod(tpl, "getRollbackPos", GetRollbackPosition);
  /*Debug - end*/

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("RingBuffer").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

RingBuffer::RingBuffer(int value) : size_(value) {
    if(value % 2 != 0) {
        std::cout << "need to be divisible by 2 to work properly" << std::endl;
    }
    buffer_ = new char[size_];
    //for dev should be removed
    memset(buffer_,0,size_);

    ring_mask_ = size_ - 1; //128 * 1024 - 1, 0x1ffff
    rollback_ = 0;
    read_position_ = 0;
    unread_ = 0;
}

RingBuffer::~RingBuffer() {
    free(buffer_);
    buffer_ = nullptr;
}

uint8_t RingBuffer::getByte() {
    read_position_ = ring_mask_ & read_position_;
    uint8_t ret = (uint8_t)buffer_[read_position_];
    read_position_++;
    unread_--;
    return ret;
}

size_t RingBuffer::getWritePos() {
    return (read_position_ + unread_) & ring_mask_;
}

size_t RingBuffer::populate(char* buffer, size_t size) {
  size_t space_left = size_ - unread_;
  size_t write_pos = getWritePos();
  //determine number of items to copy
  size_t num_to_copy = min(size, space_left);
  if(num_to_copy > 0) {
    memcpy(buffer_ + write_pos, buffer, num_to_copy);
    unread_ += num_to_copy;
  }
  return size - num_to_copy;
}

void RingBuffer::setRollback() {
    rollback_ = read_position_;
}

void RingBuffer::doRollback() {
    if(rollback_ > read_position_) {
      //if read_position_ wrapped around
      read_position_ += size_;
  }
  size_t items_undone = read_position_ - rollback_;
  unread_ += items_undone;
  read_position_ = rollback_;
}

NAN_METHOD(RingBuffer::New) {
  if (info.IsConstructCall()) {
    int value = info[0]->IsUndefined() ? 0 : Nan::To<int>(info[0]).FromJust();
    RingBuffer *obj = new RingBuffer(value);
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 1; 
    v8::Local<v8::Value> argv[argc] = {info[0]};
    v8::Local<v8::Function> cons = Nan::New(constructor);
    info.GetReturnValue().Set(cons->NewInstance(argc, argv));
  }
}
/**
* Returns the number of uncopied elements from incomming buffer
**/
NAN_METHOD(RingBuffer::Populate) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  char* buff = (char*) node::Buffer::Data(info[0]->ToObject());
  size_t size = node::Buffer::Length(info[0]);

  info.GetReturnValue().Set((int)(obj->populate(buff,size)));
}

/**
* Returns the next read value null if no more aviable values to read
**/
NAN_METHOD(RingBuffer::ReadByte) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  if(obj->unread_ > 0) {
    info.GetReturnValue().Set((int)obj->getByte());
  }
  else {
      info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(RingBuffer::SetRollback) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  obj->setRollback();
}

NAN_METHOD(RingBuffer::DoRollback) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  obj->doRollback();
}
/* DEBUG METHODS HERE*/

/**
* Returns the full buffer object
**/
NAN_METHOD(RingBuffer::GetBuffer) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  info.GetReturnValue().Set(Nan::NewBuffer(obj->buffer_, obj->size_).ToLocalChecked());
}

/**
* Returns the read position
**/
NAN_METHOD(RingBuffer::GetReadPosition) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  info.GetReturnValue().Set((int)obj->read_position_);
}

/**
* Returns the write position
**/
NAN_METHOD(RingBuffer::GetWritePosition) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  info.GetReturnValue().Set((int)obj->getWritePos());
}

/**
* Returns the number of unread items
**/
NAN_METHOD(RingBuffer::GetUnread) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  info.GetReturnValue().Set((int)obj->unread_);
}
/**
* Returns the rollback position
**/
NAN_METHOD(RingBuffer::GetRollbackPosition) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  info.GetReturnValue().Set((int)obj->rollback_);
}