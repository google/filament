#include "napi.h"

using namespace Napi;

Value getNapiVersion(const CallbackInfo& info) {
  Napi::Env env = info.Env();
  uint32_t napi_version = VersionManagement::GetNapiVersion(env);
  return Number::New(env, napi_version);
}

Value getNodeVersion(const CallbackInfo& info) {
  Napi::Env env = info.Env();
  const napi_node_version* node_version =
      VersionManagement::GetNodeVersion(env);
  Object version = Object::New(env);
  version.Set("major", Number::New(env, node_version->major));
  version.Set("minor", Number::New(env, node_version->minor));
  version.Set("patch", Number::New(env, node_version->patch));
  version.Set("release", String::New(env, node_version->release));
  return version;
}

Object InitVersionManagement(Env env) {
  Object exports = Object::New(env);
  exports["getNapiVersion"] = Function::New(env, getNapiVersion);
  exports["getNodeVersion"] = Function::New(env, getNodeVersion);
  return exports;
}
