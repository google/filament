#include <cstdlib>
#include "napi.h"
#include "test_helper.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

void CallJS(napi_env env, napi_value /* callback */, void* /*data*/) {
  Napi::Error error = Napi::Error::New(env, "test-from-native");
  NAPI_THROW_VOID(error);
}

void TestCall(const CallbackInfo& info) {
  Napi::Env env = info.Env();

  ThreadSafeFunction wrapped =
      ThreadSafeFunction::New(env,
                              info[0].As<Napi::Function>(),
                              Object::New(env),
                              String::New(env, "Test"),
                              0,
                              1);
  wrapped.BlockingCall(static_cast<void*>(nullptr));
  wrapped.Release();
}

void TestCallWithNativeCallback(const CallbackInfo& info) {
  Napi::Env env = info.Env();

  ThreadSafeFunction wrapped = ThreadSafeFunction::New(
      env, Napi::Function(), Object::New(env), String::New(env, "Test"), 0, 1);
  wrapped.BlockingCall(static_cast<void*>(nullptr), CallJS);
  wrapped.Release();
}

}  // namespace

Object InitThreadSafeFunctionException(Env env) {
  Object exports = Object::New(env);
  exports["testCall"] = Function::New(env, TestCall);
  exports["testCallWithNativeCallback"] =
      Function::New(env, TestCallWithNativeCallback);

  return exports;
}

#endif
