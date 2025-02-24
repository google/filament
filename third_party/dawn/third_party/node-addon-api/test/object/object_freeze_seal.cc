#include "napi.h"
#include "test_helper.h"

#if (NAPI_VERSION > 7)

using namespace Napi;

Value Freeze(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  return Boolean::New(info.Env(), MaybeUnwrapOr(obj.Freeze(), false));
}

Value Seal(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  return Boolean::New(info.Env(), MaybeUnwrapOr(obj.Seal(), false));
}

Object InitObjectFreezeSeal(Env env) {
  Object exports = Object::New(env);
  exports["freeze"] = Function::New(env, Freeze);
  exports["seal"] = Function::New(env, Seal);
  return exports;
}

#endif
