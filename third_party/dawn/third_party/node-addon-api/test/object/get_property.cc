#include "napi.h"
#include "test_helper.h"

using namespace Napi;

Value GetPropertyWithNapiValue(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Name key = info[1].As<Name>();
  return MaybeUnwrapOr(obj.Get(static_cast<napi_value>(key)), Value());
}

Value GetPropertyWithNapiWrapperValue(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Name key = info[1].As<Name>();
  return MaybeUnwrapOr(obj.Get(key), Value());
}

Value GetPropertyWithUint32(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Number key = info[1].As<Number>();
  return MaybeUnwrap(obj.Get(key.Uint32Value()));
}

Value GetPropertyWithCStyleString(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String jsKey = info[1].As<String>();
  return MaybeUnwrapOr(obj.Get(jsKey.Utf8Value().c_str()), Value());
}

Value GetPropertyWithCppStyleString(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String jsKey = info[1].As<String>();
  return MaybeUnwrapOr(obj.Get(jsKey.Utf8Value()), Value());
}
