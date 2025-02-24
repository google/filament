#include "napi.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

using TSFN = TypedThreadSafeFunction<>;

static Value Test(const CallbackInfo& info) {
  Object resource = info[0].As<Object>();
  Function cb = info[1].As<Function>();
  TSFN tsfn = TSFN::New(info.Env(), cb, resource, "Test", 1, 1);
  tsfn.Release();
  return info.Env().Undefined();
}

static Value ExtractEnvNullValue(const CallbackInfo& info) {
  return info.Env().Null();
}

}  // namespace

Object InitTypedThreadSafeFunctionPtr(Env env) {
  Object exports = Object::New(env);
  exports["test"] = Function::New(env, Test);
  exports["null"] = Function::New(env, ExtractEnvNullValue);
  return exports;
}

#endif
