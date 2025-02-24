#include <assert.h>
#include <napi.h>

namespace {

static int dtor_called = 0;

class DtorCounter {
 public:
  ~DtorCounter() {
    assert(dtor_called == 0);
    dtor_called++;
  }
};

Napi::Value GetDtorCalled(const Napi::CallbackInfo& info) {
  return Napi::Number::New(info.Env(), dtor_called);
}

class Test : public Napi::ObjectWrap<Test> {
 public:
  Test(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Test>(info) {
#ifdef NAPI_CPP_EXCEPTIONS
    throw Napi::Error::New(Env(), "Some error");
#else
    Napi::Error::New(Env(), "Some error").ThrowAsJavaScriptException();
#endif
  }

  static void Initialize(Napi::Env env, Napi::Object exports) {
    exports.Set("Test", DefineClass(env, "Test", {}));
    exports.Set("getDtorCalled", Napi::Function::New(env, GetDtorCalled));
  }

 private:
  DtorCounter dtor_counter_;
};

}  // anonymous namespace

Napi::Object InitObjectWrapRemoveWrap(Napi::Env env) {
  Napi::Object exports = Napi::Object::New(env);
  Test::Initialize(env, exports);
  return exports;
}
