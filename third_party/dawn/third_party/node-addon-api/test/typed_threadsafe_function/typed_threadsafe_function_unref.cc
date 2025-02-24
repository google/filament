#include "napi.h"
#include "test_helper.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

using TSFN = TypedThreadSafeFunction<>;
using ContextType = std::nullptr_t;
using FinalizerDataType = void;
static Value TestUnref(const CallbackInfo& info) {
  Napi::Env env = info.Env();
  Object global = env.Global();
  Object resource = info[0].As<Object>();
  Function cb = info[1].As<Function>();
  Function setTimeout = MaybeUnwrap(global.Get("setTimeout")).As<Function>();
  TSFN* tsfn = new TSFN;

  *tsfn = TSFN::New(
      info.Env(),
      cb,
      resource,
      "Test",
      1,
      1,
      nullptr,
      [tsfn](Napi::Env /* env */, FinalizerDataType*, ContextType*) {
        delete tsfn;
      },
      static_cast<FinalizerDataType*>(nullptr));

  tsfn->BlockingCall();
  tsfn->Ref(info.Env());

  setTimeout.Call(
      global,
      {Function::New(
           env, [tsfn](const CallbackInfo& info) { tsfn->Unref(info.Env()); }),
       Number::New(env, 100)});

  return info.Env().Undefined();
}

static Value TestRef(const CallbackInfo& info) {
  Function cb = info[1].As<Function>();

  auto tsfn = TSFN::New(info.Env(), cb, "testRes", 1, 1, nullptr);

  tsfn.BlockingCall();
  tsfn.Unref(info.Env());
  tsfn.Ref(info.Env());

  return info.Env().Undefined();
}

}  // namespace

Object InitTypedThreadSafeFunctionUnref(Env env) {
  Object exports = Object::New(env);
  exports["testUnref"] = Function::New(env, TestUnref);
  exports["testRef"] = Function::New(env, TestRef);
  return exports;
}

#endif
