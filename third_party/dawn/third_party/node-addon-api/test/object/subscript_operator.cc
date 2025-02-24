#include "napi.h"
#include "test_helper.h"

using namespace Napi;

Value SubscriptGetWithCStyleString(const CallbackInfo& info) {
  String jsKey = info[1].As<String>();

  // make sure const case compiles
  const Object obj2 = info[0].As<Object>();
  MaybeUnwrap(obj2[jsKey.Utf8Value().c_str()]).As<Boolean>();

  Object obj = info[0].As<Object>();
  return obj[jsKey.Utf8Value().c_str()];
}

Value SubscriptGetWithCppStyleString(const CallbackInfo& info) {
  String jsKey = info[1].As<String>();

  // make sure const case compiles
  const Object obj2 = info[0].As<Object>();
  MaybeUnwrap(obj2[jsKey.Utf8Value()]).As<Boolean>();

  Object obj = info[0].As<Object>();
  return obj[jsKey.Utf8Value()];
}

Value SubscriptGetAtIndex(const CallbackInfo& info) {
  uint32_t index = info[1].As<Napi::Number>();

  // make sure const case compiles
  const Object obj2 = info[0].As<Object>();
  MaybeUnwrap(obj2[index]).As<Boolean>();

  Object obj = info[0].As<Object>();
  return obj[index];
}

void SubscriptSetWithCStyleString(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String jsKey = info[1].As<String>();
  Value value = info[2];
  obj[jsKey.Utf8Value().c_str()] = value;
}

void SubscriptSetWithCppStyleString(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String jsKey = info[1].As<String>();
  Value value = info[2];
  obj[jsKey.Utf8Value()] = value;
}

void SubscriptSetAtIndex(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  uint32_t index = info[1].As<Napi::Number>();
  Value value = info[2];
  obj[index] = value;
}
