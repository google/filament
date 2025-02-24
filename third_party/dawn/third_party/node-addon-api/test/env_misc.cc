#include "napi.h"
#include "test_helper.h"

#if (NAPI_VERSION > 8)

using namespace Napi;

namespace {

Value GetModuleFileName(const CallbackInfo& info) {
  Env env = info.Env();
  return String::New(env, env.GetModuleFileName());
}

}  // end anonymous namespace

Object InitEnvMiscellaneous(Env env) {
  Object exports = Object::New(env);

  exports["get_module_file_name"] = Function::New(env, GetModuleFileName);

  return exports;
}

#endif
