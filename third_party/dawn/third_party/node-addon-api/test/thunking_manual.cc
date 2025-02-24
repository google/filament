#include <napi.h>

// The formulaic comment below should accompany any code that results in an
// internal piece of heap data getting created, because each such piece of heap
// data must be attached to an object by way of a deleter which gets called when
// the object gets garbage-collected.
//
// At the very least, you can add a fprintf(stderr, ...) to the deleter in
// napi-inl.h and then count the number of times the deleter prints by running
// node --expose-gc test/thunking_manual.js and counting the number of prints
// between the two rows of dashes. That number should coincide with the number
// of formulaic comments below.
//
// Note that currently this result can only be achieved with node-chakracore,
// because V8 does not garbage-collect classes.

static Napi::Value TestMethod(const Napi::CallbackInfo& /*info*/) {
  return Napi::Value();
}

static Napi::Value TestGetter(const Napi::CallbackInfo& /*info*/) {
  return Napi::Value();
}

static void TestSetter(const Napi::CallbackInfo& /*info*/) {}

class TestClass : public Napi::ObjectWrap<TestClass> {
 public:
  TestClass(const Napi::CallbackInfo& info) : ObjectWrap<TestClass>(info) {}
  static Napi::Value TestClassStaticMethod(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), 42);
  }

  static void TestClassStaticVoidMethod(const Napi::CallbackInfo& /*info*/) {}

  Napi::Value TestClassInstanceMethod(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), 42);
  }

  void TestClassInstanceVoidMethod(const Napi::CallbackInfo& /*info*/) {}

  Napi::Value TestClassInstanceGetter(const Napi::CallbackInfo& info) {
    return Napi::Number::New(info.Env(), 42);
  }

  void TestClassInstanceSetter(const Napi::CallbackInfo& /*info*/,
                               const Napi::Value& /*new_value*/) {}

  static Napi::Function NewClass(Napi::Env env) {
    return DefineClass(
        env,
        "TestClass",
        {// Make sure to check that the deleter gets called.
         StaticMethod("staticMethod", TestClassStaticMethod),
         // Make sure to check that the deleter gets called.
         StaticMethod("staticVoidMethod", TestClassStaticVoidMethod),
         // Make sure to check that the deleter gets called.
         StaticMethod(Napi::Symbol::New(env, "staticMethod"),
                      TestClassStaticMethod),
         // Make sure to check that the deleter gets called.
         StaticMethod(Napi::Symbol::New(env, "staticVoidMethod"),
                      TestClassStaticVoidMethod),
         // Make sure to check that the deleter gets called.
         InstanceMethod("instanceMethod", &TestClass::TestClassInstanceMethod),
         // Make sure to check that the deleter gets called.
         InstanceMethod("instanceVoidMethod",
                        &TestClass::TestClassInstanceVoidMethod),
         // Make sure to check that the deleter gets called.
         InstanceMethod(Napi::Symbol::New(env, "instanceMethod"),
                        &TestClass::TestClassInstanceMethod),
         // Make sure to check that the deleter gets called.
         InstanceMethod(Napi::Symbol::New(env, "instanceVoidMethod"),
                        &TestClass::TestClassInstanceVoidMethod),
         // Make sure to check that the deleter gets called.
         InstanceAccessor("instanceAccessor",
                          &TestClass::TestClassInstanceGetter,
                          &TestClass::TestClassInstanceSetter)});
  }
};

static Napi::Value CreateTestObject(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object item = Napi::Object::New(env);

  // Make sure to check that the deleter gets called.
  item["testMethod"] = Napi::Function::New(env, TestMethod, "testMethod");

  item.DefineProperties({
      // Make sure to check that the deleter gets called.
      Napi::PropertyDescriptor::Accessor(env, item, "accessor_1", TestGetter),
      // Make sure to check that the deleter gets called.
      Napi::PropertyDescriptor::Accessor(
          env, item, std::string("accessor_1_std_string"), TestGetter),
      // Make sure to check that the deleter gets called.
      Napi::PropertyDescriptor::Accessor(
          env,
          item,
          Napi::String::New(info.Env(), "accessor_1_js_string"),
          TestGetter),
      // Make sure to check that the deleter gets called.
      Napi::PropertyDescriptor::Accessor(
          env, item, "accessor_2", TestGetter, TestSetter),
      // Make sure to check that the deleter gets called.
      Napi::PropertyDescriptor::Accessor(env,
                                         item,
                                         std::string("accessor_2_std_string"),
                                         TestGetter,
                                         TestSetter),
      // Make sure to check that the deleter gets called.
      Napi::PropertyDescriptor::Accessor(
          env,
          item,
          Napi::String::New(env, "accessor_2_js_string"),
          TestGetter,
          TestSetter),
      Napi::PropertyDescriptor::Value("TestClass", TestClass::NewClass(env)),
  });

  return item;
}

Napi::Object InitThunkingManual(Napi::Env env) {
  Napi::Object exports = Napi::Object::New(env);
  exports["createTestObject"] =
      Napi::Function::New(env, CreateTestObject, "createTestObject");
  return exports;
}
