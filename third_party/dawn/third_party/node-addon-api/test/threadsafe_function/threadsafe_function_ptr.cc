#include "napi.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

static Value Test(const CallbackInfo& info) {
  Object resource = info[0].As<Object>();
  Function cb = info[1].As<Function>();
  ThreadSafeFunction tsfn =
      ThreadSafeFunction::New(info.Env(), cb, resource, "Test", 1, 1);
  tsfn.Release();
  return info.Env().Undefined();
}

}  // namespace

Object InitThreadSafeFunctionPtr(Env env) {
  Object exports = Object::New(env);
  exports["test"] = Function::New(env, Test);

  return exports;
}

#endif
