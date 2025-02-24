#include "napi.h"

using namespace Napi;

Object InitError(Env env);

Object Init(Env env, Object exports) {
  exports.Set("error", InitError(env));
  return exports;
}

NODE_API_MODULE(addon, Init)
