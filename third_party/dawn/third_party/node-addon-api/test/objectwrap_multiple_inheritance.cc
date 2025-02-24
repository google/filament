#include <napi.h>

class TestMIBase {
 public:
  TestMIBase() : test(0) {}
  virtual void dummy() {}
  uint32_t test;
};

class TestMI : public TestMIBase, public Napi::ObjectWrap<TestMI> {
 public:
  TestMI(const Napi::CallbackInfo& info) : Napi::ObjectWrap<TestMI>(info) {}

  Napi::Value GetTest(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), test);
  }

  static void Initialize(Napi::Env env, Napi::Object exports) {
    exports.Set(
        "TestMI",
        DefineClass(
            env, "TestMI", {InstanceAccessor<&TestMI::GetTest>("test")}));
  }
};

Napi::Object InitObjectWrapMultipleInheritance(Napi::Env env) {
  Napi::Object exports = Napi::Object::New(env);
  TestMI::Initialize(env, exports);
  return exports;
}
