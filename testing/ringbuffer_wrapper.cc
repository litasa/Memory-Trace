#include "ringbuffer.h"

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

  info.GetReturnValue().Set((uint32_t)(obj->populate(buff,size)));
}

/**
* Returns the next read value null if no more aviable values to read
**/
NAN_METHOD(RingBuffer::ReadByte) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  if(obj->unread_ > 0) {
    info.GetReturnValue().Set((uint32_t)obj->getByte());
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
  info.GetReturnValue().Set((uint32_t)obj->read_position_);
}

/**
* Returns the write position
**/
NAN_METHOD(RingBuffer::GetWritePosition) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  info.GetReturnValue().Set((uint32_t)obj->getWritePos());
}

/**
* Returns the number of unread items
**/
NAN_METHOD(RingBuffer::GetUnread) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  info.GetReturnValue().Set((uint32_t)obj->unread_);
}
/**
* Returns the rollback position
**/
NAN_METHOD(RingBuffer::GetRollbackPosition) {
  RingBuffer* obj = Nan::ObjectWrap::Unwrap<RingBuffer>(info.This());
  info.GetReturnValue().Set((uint32_t)obj->rollback_);
}