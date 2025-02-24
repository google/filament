#include "napi.h"
#include "test_helper.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

static Value TestUnref(const CallbackInfo& info) {
  Napi::Env env = info.Env();
  Object global = env.Global();
  Object resource = info[0].As<Object>();
  Function cb = info[1].As<Function>();
  Function setTimeout = MaybeUnwrap(global.Get("setTimeout")).As<Function>();
  ThreadSafeFunction* tsfn = new ThreadSafeFunction;

  *tsfn = ThreadSafeFunction::New(
      info.Env(), cb, resource, "Test", 1, 1, [tsfn](Napi::Env /* env */) {
        delete tsfn;
      });

  tsfn->BlockingCall();

  setTimeout.Call(
      global,
      {Function::New(
           env, [tsfn](const CallbackInfo& info) { tsfn->Unref(info.Env()); }),
       Number::New(env, 100)});

  return info.Env().Undefined();
}

static Value TestRef(const CallbackInfo& info) {
  Function cb = info[1].As<Function>();

  auto tsfn = ThreadSafeFunction::New(info.Env(), cb, "testRes", 1, 1);

  tsfn.BlockingCall();
  tsfn.Unref(info.Env());
  tsfn.Ref(info.Env());

  return info.Env().Undefined();
}

}  // namespace

Object InitThreadSafeFunctionUnref(Env env) {
  Object exports = Object::New(env);
  exports["testUnref"] = Function::New(env, TestUnref);
  exports["testRef"] = Function::New(env, TestRef);
  return exports;
}

#endif
