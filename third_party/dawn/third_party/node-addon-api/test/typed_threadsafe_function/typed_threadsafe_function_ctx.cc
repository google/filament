#include <assert.h>
#include "napi.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

using ContextType = Reference<Napi::Value>;
using TSFN = TypedThreadSafeFunction<ContextType>;

namespace {

class TSFNWrap : public ObjectWrap<TSFNWrap> {
 public:
  static Function Init(Napi::Env env);
  TSFNWrap(const CallbackInfo& info);

  Napi::Value GetContext(const CallbackInfo& /*info*/) {
    ContextType* ctx = _tsfn.GetContext();
    return ctx->Value();
  };

  Napi::Value Release(const CallbackInfo& info) {
    Napi::Env env = info.Env();
    _deferred = std::unique_ptr<Promise::Deferred>(new Promise::Deferred(env));
    _tsfn.Release();
    return _deferred->Promise();
  };

 private:
  TSFN _tsfn;
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
  ContextType* _ctx = new ContextType;
  *_ctx = Persistent(info[0]);

  _tsfn = TSFN::New(info.Env(),
                    this->Value(),
                    "Test",
                    1,
                    1,
                    _ctx,
                    [this](Napi::Env env, void*, ContextType* ctx) {
                      _deferred->Resolve(env.Undefined());
                      ctx->Reset();
                      delete ctx;
                    });
}

}  // namespace

struct SimpleTestContext {
  SimpleTestContext(int val) : _val(val) {}
  int _val = -1;
};

// A simple test to check that the context has been set successfully
void AssertGetContextFromTSFNNoFinalizerIsCorrect(const CallbackInfo& info) {
  // Test the overload where we provide a resource name but no finalizer
  using TSFN = TypedThreadSafeFunction<SimpleTestContext>;
  SimpleTestContext* ctx = new SimpleTestContext(42);
  TSFN tsfn = TSFN::New(info.Env(), "testRes", 1, 1, ctx);

  assert(tsfn.GetContext() == ctx);
  delete ctx;
  tsfn.Release();

  // Test the other overload where we provide a async resource object, res name
  // but no finalizer
  ctx = new SimpleTestContext(52);
  tsfn = TSFN::New(
      info.Env(), Object::New(info.Env()), "testResourceObject", 1, 1, ctx);

  assert(tsfn.GetContext() == ctx);
  delete ctx;
  tsfn.Release();

  ctx = new SimpleTestContext(52);
  tsfn = TSFN::New(info.Env(),
                   "resStrings",
                   1,
                   1,
                   ctx,
                   [](Napi::Env, void*, SimpleTestContext*) {});

  assert(tsfn.GetContext() == ctx);
  delete ctx;
  tsfn.Release();

  ctx = new SimpleTestContext(52);
  Function emptyFunc;
  tsfn = TSFN::New(info.Env(), emptyFunc, "resString", 1, 1, ctx);
  assert(tsfn.GetContext() == ctx);
  delete ctx;
  tsfn.Release();
}

Object InitTypedThreadSafeFunctionCtx(Env env) {
  Object exports = Object::New(env);
  Function tsfnWrap = TSFNWrap::Init(env);

  exports.Set("TSFNWrap", tsfnWrap);
  exports.Set("AssertTSFNReturnCorrectCxt",
              Function::New(env, AssertGetContextFromTSFNNoFinalizerIsCorrect));
  return exports;
}

#endif
