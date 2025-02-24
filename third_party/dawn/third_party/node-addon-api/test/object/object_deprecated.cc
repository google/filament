#include "napi.h"

using namespace Napi;

static bool testValue = true;

namespace {

Value TestGetter(const CallbackInfo& info) {
  return Boolean::New(info.Env(), testValue);
}

void TestSetter(const CallbackInfo& info) {
  testValue = info[0].As<Boolean>();
}

Value TestFunction(const CallbackInfo& info) {
  return Boolean::New(info.Env(), true);
}

void DefineProperties(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String nameType = info[1].As<String>();
  Env env = info.Env();

  if (nameType.Utf8Value() == "literal") {
    obj.DefineProperties({
        PropertyDescriptor::Accessor("readonlyAccessor", TestGetter),
        PropertyDescriptor::Accessor(
            "readwriteAccessor", TestGetter, TestSetter),
        PropertyDescriptor::Function("function", TestFunction),
    });
  } else if (nameType.Utf8Value() == "string") {
    // VS2013 has lifetime issues when passing temporary objects into the
    // constructor of another object. It generates code to destruct the object
    // as soon as the constructor call returns. Since this isn't a common case
    // for using std::string objects, I'm refactoring the test to work around
    // the issue.
    std::string str1("readonlyAccessor");
    std::string str2("readwriteAccessor");
    std::string str7("function");

    obj.DefineProperties({
        PropertyDescriptor::Accessor(str1, TestGetter),
        PropertyDescriptor::Accessor(str2, TestGetter, TestSetter),
        PropertyDescriptor::Function(str7, TestFunction),
    });
  } else if (nameType.Utf8Value() == "value") {
    obj.DefineProperties({
        PropertyDescriptor::Accessor(Napi::String::New(env, "readonlyAccessor"),
                                     TestGetter),
        PropertyDescriptor::Accessor(
            Napi::String::New(env, "readwriteAccessor"),
            TestGetter,
            TestSetter),
        PropertyDescriptor::Function(Napi::String::New(env, "function"),
                                     TestFunction),
    });
  }
}

}  // end of anonymous namespace

Object InitObjectDeprecated(Env env) {
  Object exports = Object::New(env);

  exports["defineProperties"] = Function::New(env, DefineProperties);

  return exports;
}
