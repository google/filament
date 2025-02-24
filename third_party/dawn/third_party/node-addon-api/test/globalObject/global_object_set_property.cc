#include "napi.h"

using namespace Napi;

void SetPropertyWithCStyleStringAsKey(const CallbackInfo& info) {
  Object globalObject = info.Env().Global();
  String key = info[0].As<String>();
  Value value = info[1];
  globalObject.Set(key.Utf8Value().c_str(), value);
}

void SetPropertyWithCppStyleStringAsKey(const CallbackInfo& info) {
  Object globalObject = info.Env().Global();
  String key = info[0].As<String>();
  Value value = info[1];
  globalObject.Set(key.Utf8Value(), value);
}

void SetPropertyWithInt32AsKey(const CallbackInfo& info) {
  Object globalObject = info.Env().Global();
  Number key = info[0].As<Number>();
  Value value = info[1];
  globalObject.Set(key.Uint32Value(), value);
}

void SetPropertyWithNapiValueAsKey(const CallbackInfo& info) {
  Object globalObject = info.Env().Global();
  Name key = info[0].As<Name>();
  Value value = info[1];
  globalObject.Set(key, value);
}