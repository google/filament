#if (NAPI_VERSION > 5)
#include <stdio.h>
#include "napi.h"

namespace {

class TestAddon : public Napi::Addon<TestAddon> {
 public:
  inline TestAddon(Napi::Env env, Napi::Object exports) {
    DefineAddon(
        exports,
        {InstanceMethod("increment", &TestAddon::Increment),
         InstanceValue(
             "subObject",
             DefineProperties(
                 Napi::Object::New(env),
                 {InstanceMethod("decrement", &TestAddon::Decrement)}))});
  }

  ~TestAddon() { fprintf(stderr, "TestAddon::~TestAddon\n"); }

 private:
  Napi::Value Increment(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), ++value);
  }

  Napi::Value Decrement(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), --value);
  }

  uint32_t value = 42;
};

Napi::Value CreateAddon(const Napi::CallbackInfo& info) {
  return TestAddon::Init(info.Env(), Napi::Object::New(info.Env()));
}

}  // end of anonymous namespace

Napi::Object InitAddon(Napi::Env env) {
  return Napi::Function::New<CreateAddon>(env, "CreateAddon");
}

#endif  // (NAPI_VERSION > 5)
