#include <napi.h>

Napi::Value Echo(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (info.Length() != 1) {
    Napi::TypeError::New(env,
                         "Wrong number of arguments. One argument expected.")
        .ThrowAsJavaScriptException();
  }
  return info[0].As<Napi::Value>();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "echo"), Napi::Function::New(env, Echo));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
