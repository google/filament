#include <assert.h>
#include "napi.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

class TSFNWrap : public ObjectWrap<TSFNWrap> {
 public:
  static Function Init(Napi::Env env);
  TSFNWrap(const CallbackInfo& info);

  Napi::Value GetContext(const CallbackInfo& /*info*/) {
    Reference<Napi::Value>* ctx = _tsfn.GetContext();
    return ctx->Value();
  };

  Napi::Value Release(const CallbackInfo& info) {
    Napi::Env env = info.Env();
    _deferred = std::unique_ptr<Promise::Deferred>(new Promise::Deferred(env));
    _tsfn.Release();
    return _deferred->Promise();
  };

 private:
  ThreadSafeFunction _tsfn;
  std::unique_ptr<Promise::Deferred> _deferred;
};

Function TSFNWrap::Init(Napi::Env env) {
  Function func =
      DefineClass(env,
                  "TSFNWrap",
                  {InstanceMethod("getContext", &TSFNWrap::GetContext),
                   InstanceMethod("release", &TSFNWrap::Release)});
  return func;
}

TSFNWrap::TSFNWrap(const CallbackInfo& info) : ObjectWrap<TSFNWrap>(info) {
  Napi::Env env = info.Env();

  Reference<Napi::Value>* _ctx = new Reference<Napi::Value>;
  *_ctx = Persistent(info[0]);

  _tsfn = ThreadSafeFunction::New(
      info.Env(),
      Function::New(env, [](const CallbackInfo& /*info*/) {}),
      Object::New(env),
      "Test",
      1,
      1,
      _ctx,
      [this](Napi::Env env, Reference<Napi::Value>* ctx) {
        _deferred->Resolve(env.Undefined());
        ctx->Reset();
        delete ctx;
      });
}
struct SimpleTestContext {
  SimpleTestContext(int val) : _val(val) {}
  int _val = -1;
};

void AssertGetContextFromVariousTSOverloads(const CallbackInfo& info) {
  Env env = info.Env();
  Function emptyFunc;

  SimpleTestContext* ctx = new SimpleTestContext(42);
  ThreadSafeFunction fn =
      ThreadSafeFunction::New(env, emptyFunc, "testResource", 1, 1, ctx);

  assert(fn.GetContext() == ctx);
  delete ctx;
  fn.Release();

  fn = ThreadSafeFunction::New(env, emptyFunc, "testRes", 1, 1, [](Env) {});
  fn.Release();

  ctx = new SimpleTestContext(42);
  fn = ThreadSafeFunction::New(env,
                               emptyFunc,
                               Object::New(env),
                               "resStrObj",
                               1,
                               1,
                               ctx,
                               [](Env, SimpleTestContext*) {});
  assert(fn.GetContext() == ctx);
  delete ctx;
  fn.Release();

  fn = ThreadSafeFunction::New(
      env, emptyFunc, Object::New(env), "resStrObj", 1, 1);
  fn.Release();

  ctx = new SimpleTestContext(42);
  fn = ThreadSafeFunction::New(
      env, emptyFunc, Object::New(env), "resStrObj", 1, 1, ctx);
  assert(fn.GetContext() == ctx);
  delete ctx;
  fn.Release();

  using FinalizerDataType = int;
  FinalizerDataType* finalizerData = new int(42);
  fn = ThreadSafeFunction::New(
      env,
      emptyFunc,
      Object::New(env),
      "resObject",
      1,
      1,
      [](Env, FinalizerDataType* data) {
        assert(*data == 42);
        delete data;
      },
      finalizerData);
  fn.Release();

  ctx = new SimpleTestContext(42);
  FinalizerDataType* finalizerDataB = new int(42);

  fn = ThreadSafeFunction::New(
      env,
      emptyFunc,
      Object::New(env),
      "resObject",
      1,
      1,
      ctx,
      [](Env, FinalizerDataType* _data, SimpleTestContext* _ctx) {
        assert(*_data == 42);
        assert(_ctx->_val == 42);
        delete _data;
        delete _ctx;
      },
      finalizerDataB);
  assert(fn.GetContext() == ctx);
  fn.Release();
}

}  // namespace

Object InitThreadSafeFunctionCtx(Env env) {
  Object exports = Object::New(env);
  Function tsfnWrap = TSFNWrap::Init(env);
  exports.Set("TSFNWrap", tsfnWrap);
  exports.Set("AssertFnReturnCorrectCxt",
              Function::New(env, AssertGetContextFromVariousTSOverloads));

  return exports;
}

#endif
