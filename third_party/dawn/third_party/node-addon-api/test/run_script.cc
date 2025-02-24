#include "napi.h"
#include "test_helper.h"

using namespace Napi;

namespace {

Value RunPlainString(const CallbackInfo& info) {
  Env env = info.Env();
  return MaybeUnwrap(env.RunScript("1 + 2 + 3"));
}

Value RunStdString(const CallbackInfo& info) {
  Env env = info.Env();
  std::string str = "1 + 2 + 3";
  return MaybeUnwrap(env.RunScript(str));
}

Value RunJsString(const CallbackInfo& info) {
  Env env = info.Env();
  return MaybeUnwrapOr(env.RunScript(info[0].As<String>()), Value());
}

Value RunWithContext(const CallbackInfo& info) {
  Env env = info.Env();

  Array keys = MaybeUnwrap(info[1].As<Object>().GetPropertyNames());
  std::string code = "(";
  for (unsigned int i = 0; i < keys.Length(); i++) {
    if (i != 0) code += ",";
    code += MaybeUnwrap(keys.Get(i)).As<String>().Utf8Value();
  }
  code += ") => " + info[0].As<String>().Utf8Value();

  Value ret = MaybeUnwrap(env.RunScript(code));
  Function fn = ret.As<Function>();
  std::vector<napi_value> args;
  for (unsigned int i = 0; i < keys.Length(); i++) {
    Value key = MaybeUnwrap(keys.Get(i));
    args.push_back(MaybeUnwrap(info[1].As<Object>().Get(key)));
  }
  return MaybeUnwrap(fn.Call(args));
}

}  // end anonymous namespace

Object InitRunScript(Env env) {
  Object exports = Object::New(env);

  exports["plainString"] = Function::New(env, RunPlainString);
  exports["stdString"] = Function::New(env, RunStdString);
  exports["jsString"] = Function::New(env, RunJsString);
  exports["withContext"] = Function::New(env, RunWithContext);

  return exports;
}
