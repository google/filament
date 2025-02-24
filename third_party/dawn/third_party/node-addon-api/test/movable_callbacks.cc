#include "napi.h"

using namespace Napi;

Value createExternal(const CallbackInfo& info) {
  FunctionReference ref = Reference<Function>::New(info[0].As<Function>(), 1);
  auto ret = External<char>::New(
      info.Env(),
      nullptr,
      [ref = std::move(ref)](Napi::Env /*env*/, char* /*data*/) {
        ref.Call({});
      });

  return ret;
}

Object InitMovableCallbacks(Env env) {
  Object exports = Object::New(env);

  exports["createExternal"] = Function::New(env, createExternal);

  return exports;
}
