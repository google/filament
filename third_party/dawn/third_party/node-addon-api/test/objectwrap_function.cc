#include <napi.h>
#include <unordered_map>
#include "test_helper.h"

class FunctionTest : public Napi::ObjectWrap<FunctionTest> {
 public:
  FunctionTest(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<FunctionTest>(info) {}

  static Napi::Value OnCalledAsFunction(const Napi::CallbackInfo& info) {
    // If called with a "true" argument, throw an exeption to test the handling.
    if (!info[0].IsUndefined() && MaybeUnwrap(info[0].ToBoolean())) {
      NAPI_THROW(Napi::Error::New(info.Env(), "an exception"), Napi::Value());
    }
    // Otherwise, act as a factory.
    std::vector<napi_value> args;
    for (size_t i = 0; i < info.Length(); i++) args.push_back(info[i]);
    return MaybeUnwrap(GetConstructor(info.Env()).New(args));
  }

  static void Initialize(Napi::Env env, Napi::Object exports) {
    const char* name = "FunctionTest";
    Napi::Function func = DefineClass(env, name, {});
    Napi::FunctionReference* ctor = new Napi::FunctionReference();
    *ctor = Napi::Persistent(func);
    env.SetInstanceData(ctor);
    exports.Set(name, func);
  }

  static Napi::Function GetConstructor(Napi::Env env) {
    return env.GetInstanceData<Napi::FunctionReference>()->Value();
  }
};

Napi::Value ObjectWrapFunctionFactory(const Napi::CallbackInfo& info) {
  Napi::Object exports = Napi::Object::New(info.Env());
  FunctionTest::Initialize(info.Env(), exports);
  return exports;
}

Napi::Object InitObjectWrapFunction(Napi::Env env) {
  return Napi::Function::New<ObjectWrapFunctionFactory>(env, "FunctionFactory");
}
