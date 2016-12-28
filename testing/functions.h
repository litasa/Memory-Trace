#ifndef NATIVE_EXTENSION_GRAB_H
#define NATIVE_EXTENSION_GRAB_H

#include <nan.h>
#include "ringbuffer.h"

// Example top-level functions. These functions demonstrate how to return various js types.
// Implementations are in functions.cc

NAN_METHOD(DecodeValue);
NAN_METHOD(DecodeString);
NAN_METHOD(EncodeString);
NAN_METHOD(EncodeValue);
NAN_METHOD(EncodeNumber);
NAN_METHOD(DecodeStream);
#endif
