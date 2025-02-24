#include <napi.h>

class ConstructorExceptionTest
    : public Napi::ObjectWrap<ConstructorExceptionTest> {
 public:
  ConstructorExceptionTest(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<ConstructorExceptionTest>(info) {
    Napi::Error error = Napi::Error::New(info.Env(), "an exception");
#ifdef NAPI_DISABLE_CPP_EXCEPTIONS
    error.ThrowAsJavaScriptException();
#else
    throw error;
#endif  // NAPI_DISABLE_CPP_EXCEPTIONS
  }

  static void Initialize(Napi::Env env, Napi::Object exports) {
    const char* name = "ConstructorExceptionTest";
    exports.Set(name, DefineClass(env, name, {}));
  }
};

Napi::Object InitObjectWrapConstructorException(Napi::Env env) {
  Napi::Object exports = Napi::Object::New(env);
  ConstructorExceptionTest::Initialize(env, exports);
  return exports;
}
