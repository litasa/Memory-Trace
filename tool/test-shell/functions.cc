#include "functions.h"
#include <string>

NAN_METHOD(nothing) {
}

NAN_METHOD(aString) {
    info.GetReturnValue().Set(Nan::New("This is a thing.").ToLocalChecked());
}

NAN_METHOD(aBoolean) {
    info.GetReturnValue().Set(false);
}

NAN_METHOD(aNumber) {
    unsigned int val = info[0]->Uint32Value();
    info.GetReturnValue().Set(1.75);
}

NAN_METHOD(anObject) {
    v8::Local<v8::Object> obj = Nan::New<v8::Object>();
    Nan::Set(obj, Nan::New("key").ToLocalChecked(), Nan::New("value").ToLocalChecked());
    info.GetReturnValue().Set(obj);
}

NAN_METHOD(anArray) {
    v8::Local<v8::Array> arr = Nan::New<v8::Array>(3);
    Nan::Set(arr, 0, Nan::New(1));
    Nan::Set(arr, 1, Nan::New(2));
    Nan::Set(arr, 2, Nan::New(3));
    info.GetReturnValue().Set(arr);
}

NAN_METHOD(callback) {
    v8::Local<v8::Function> callbackHandle = info[0].As<v8::Function>();
    Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callbackHandle, 0, 0);
}

NAN_METHOD(decode) {
    char* buffer = (char*) node::Buffer::Data(info[0]->ToObject());
    size_t size = node::Buffer::Length(info[0]);

	uint64_t mul = 1;
	uint64_t val = 0;
    int count = 0;
	for (int i = 0; i < size; ++i) {
		uint8_t b = (uint8_t)buffer[i];
		
		val = val | (b*mul);
		mul = mul << 7;
        count++;
	}
    char * retval = new char[size]; 
	val &= ~mul;
	for (int i = 0; i < size; ++i) {
		char var = val & 0xff;
		retval[i] = var;
		val = val >> 8;
	}
   
    info.GetReturnValue().Set(Nan::NewBuffer(retval, size).ToLocalChecked());
}
// Wrapper Impl

Nan::Persistent<v8::Function> MyObject::constructor;

NAN_MODULE_INIT(MyObject::Init) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->SetClassName(Nan::New("MyObject").ToLocalChecked());
  
  Nan::SetPrototypeMethod(tpl, "plusOne", PlusOne);
  Nan::SetPrototypeMethod(tpl, "minusOne", MinusOne);
  Nan::SetPrototypeMethod(tpl, "getName", GetName);

  constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
  Nan::Set(target, Nan::New("MyObject").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

MyObject::MyObject(double value, std::string name) : value_(value), name_(name) {
}

MyObject::~MyObject() {
}

NAN_METHOD(MyObject::New) {
    if (!info[1]->IsString()) {
        // This clause would catch IsNull and IsUndefined too...
        return ;
    }
    double value = info[0]->IsUndefined() ? 0 : Nan::To<double>(info[0]).FromJust();
    v8::String::Utf8Value name(info[1]);
    MyObject *obj = new MyObject(value, std::string(*name));
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

NAN_METHOD(MyObject::PlusOne) {
  MyObject* obj = Nan::ObjectWrap::Unwrap<MyObject>(info.This());
  obj->value_ += 1;
  info.GetReturnValue().Set(obj->value_);
}

NAN_METHOD(MyObject::MinusOne) {
  MyObject* obj = Nan::ObjectWrap::Unwrap<MyObject>(info.This());
  obj->value_ -= 1;
  info.GetReturnValue().Set(obj->value_);
}

NAN_METHOD(MyObject::GetName) {
    MyObject* obj = Nan::ObjectWrap::Unwrap<MyObject>(info.This());
    v8::Local<v8::String> retval = v8::String::NewFromUtf8(info.GetIsolate(), obj->name_.c_str());
    info.GetReturnValue().Set(retval);
}