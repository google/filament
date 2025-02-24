#include <cstdlib>
#include "napi.h"
#include "test_helper.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

void CallJS(Napi::Env env,
            Napi::Function /* callback */,
            std::nullptr_t* /* context */,
            void* /*data*/) {
  Napi::Error error = Napi::Error::New(env, "test-from-native");
  NAPI_THROW_VOID(error);
}

using TSFN = TypedThreadSafeFunction<std::nullptr_t, void, CallJS>;

void TestCall(const CallbackInfo& info) {
  Napi::Env env = info.Env();

  TSFN wrapped = TSFN::New(
      env, Napi::Function(), Object::New(env), String::New(env, "Test"), 0, 1);
  wrapped.BlockingCall(static_cast<void*>(nullptr));
  wrapped.Release();
}

}  // namespace

Object InitTypedThreadSafeFunctionException(Env env) {
  Object exports = Object::New(env);
  exports["testCall"] = Function::New(env, TestCall);

  return exports;
}

#endif
