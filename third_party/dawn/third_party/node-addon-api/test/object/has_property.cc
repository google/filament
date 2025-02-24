#include "napi.h"
#include "test_helper.h"

using namespace Napi;

Value HasPropertyWithNapiValue(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Name key = info[1].As<Name>();
  return Boolean::New(
      info.Env(), MaybeUnwrapOr(obj.Has(static_cast<napi_value>(key)), false));
}

Value HasPropertyWithNapiWrapperValue(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Name key = info[1].As<Name>();
  return Boolean::New(info.Env(), MaybeUnwrapOr(obj.Has(key), false));
}

Value HasPropertyWithCStyleString(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String jsKey = info[1].As<String>();
  return Boolean::New(info.Env(),
                      MaybeUnwrapOr(obj.Has(jsKey.Utf8Value().c_str()), false));
}

Value HasPropertyWithUint32(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Number jsKey = info[1].As<Number>();
  return Boolean::New(info.Env(),
                      MaybeUnwrapOr(obj.Has(jsKey.Uint32Value()), false));
}

Value HasPropertyWithCppStyleString(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String jsKey = info[1].As<String>();
  return Boolean::New(info.Env(),
                      MaybeUnwrapOr(obj.Has(jsKey.Utf8Value()), false));
}
