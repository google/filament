#include "napi.h"
#include "test_helper.h"

using namespace Napi;

Value HasOwnPropertyWithNapiValue(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Name key = info[1].As<Name>();
  return Boolean::New(
      info.Env(),
      MaybeUnwrapOr(obj.HasOwnProperty(static_cast<napi_value>(key)), false));
}

Value HasOwnPropertyWithNapiWrapperValue(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Name key = info[1].As<Name>();
  return Boolean::New(info.Env(),
                      MaybeUnwrapOr(obj.HasOwnProperty(key), false));
}

Value HasOwnPropertyWithCStyleString(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String jsKey = info[1].As<String>();
  return Boolean::New(
      info.Env(),
      MaybeUnwrapOr(obj.HasOwnProperty(jsKey.Utf8Value().c_str()), false));
}

Value HasOwnPropertyWithCppStyleString(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String jsKey = info[1].As<String>();
  return Boolean::New(
      info.Env(), MaybeUnwrapOr(obj.HasOwnProperty(jsKey.Utf8Value()), false));
}
