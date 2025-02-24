#include "napi.h"

using namespace Napi;

namespace {

static void MakeCallback(const CallbackInfo& info) {
  Function callback = info[0].As<Function>();
  Object resource = info[1].As<Object>();
  AsyncContext context(info.Env(), "async_context_test", resource);
  callback.MakeCallback(
      Object::New(info.Env()), std::initializer_list<napi_value>{}, context);
}

static void MakeCallbackNoResource(const CallbackInfo& info) {
  Function callback = info[0].As<Function>();
  AsyncContext context(info.Env(), "async_context_no_res_test");
  callback.MakeCallback(
      Object::New(info.Env()), std::initializer_list<napi_value>{}, context);
}

static Boolean AssertAsyncContextReturnCorrectEnv(const CallbackInfo& info) {
  AsyncContext context(info.Env(), "empty_context_test");
  return Boolean::New(info.Env(), context.Env() == info.Env());
}
}  // end anonymous namespace

Object InitAsyncContext(Env env) {
  Object exports = Object::New(env);
  exports["makeCallback"] = Function::New(env, MakeCallback);
  exports["makeCallbackNoResource"] =
      Function::New(env, MakeCallbackNoResource);
  exports["asyncCxtReturnCorrectEnv"] =
      Function::New(env, AssertAsyncContextReturnCorrectEnv);
  return exports;
}
