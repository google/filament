#include <napi.h>

namespace {
void Test(const Napi::CallbackInfo& info) {
  info[0].As<Napi::Function>().Call({});
}

}  // namespace
Napi::Object InitErrorHandlingPrim(Napi::Env env) {
  Napi::Object exports = Napi::Object::New(env);
  exports.Set("errorHandlingPrim", Napi::Function::New<Test>(env));
  return exports;
}
