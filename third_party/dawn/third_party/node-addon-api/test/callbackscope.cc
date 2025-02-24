#include "assert.h"
#include "napi.h"
using namespace Napi;

#if (NAPI_VERSION > 2)

namespace {

static void RunInCallbackScope(const CallbackInfo& info) {
  Function callback = info[0].As<Function>();
  AsyncContext context(info.Env(), "callback_scope_test");
  CallbackScope scope(info.Env(), context);
  callback.Call({});
}

static void RunInCallbackScopeFromExisting(const CallbackInfo& info) {
  Function callback = info[0].As<Function>();
  Env env = info.Env();

  AsyncContext ctx(env, "existing_callback_scope_test");
  napi_callback_scope scope;
  napi_open_callback_scope(env, Object::New(env), ctx, &scope);

  CallbackScope existingScope(env, scope);
  assert(existingScope.Env() == env);

  callback.Call({});
}

}  // namespace

Object InitCallbackScope(Env env) {
  Object exports = Object::New(env);
  exports["runInCallbackScope"] = Function::New(env, RunInCallbackScope);
  exports["runInPreExistingCbScope"] =
      Function::New(env, RunInCallbackScopeFromExisting);
  return exports;
}
#endif
