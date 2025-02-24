#include "napi.h"
#include "test_helper.h"

using namespace Napi;

Value GetPropertyWithNapiValueAsKey(const CallbackInfo& info) {
  Object globalObject = info.Env().Global();
  Name key = info[0].As<Name>();
  return MaybeUnwrap(globalObject.Get(key));
}

Value GetPropertyWithInt32AsKey(const CallbackInfo& info) {
  Object globalObject = info.Env().Global();
  Number key = info[0].As<Napi::Number>();
  return MaybeUnwrapOr(globalObject.Get(key.Uint32Value()), Value());
}

Value GetPropertyWithCStyleStringAsKey(const CallbackInfo& info) {
  Object globalObject = info.Env().Global();
  String cStrkey = info[0].As<String>();
  return MaybeUnwrapOr(globalObject.Get(cStrkey.Utf8Value().c_str()), Value());
}

Value GetPropertyWithCppStyleStringAsKey(const CallbackInfo& info) {
  Object globalObject = info.Env().Global();
  String cppStrKey = info[0].As<String>();
  return MaybeUnwrapOr(globalObject.Get(cppStrKey.Utf8Value()), Value());
}

void CreateMockTestObject(const CallbackInfo& info) {
  Object globalObject = info.Env().Global();
  Number napi_key = Number::New(info.Env(), 2);
  const char* CStringKey = "c_str_key";

  globalObject.Set(napi_key, "napi_attribute");
  globalObject[CStringKey] = "c_string_attribute";
  globalObject[std::string("cpp_string_key")] = "cpp_string_attribute";
  globalObject[std::string("circular")] = globalObject;
  globalObject[(uint32_t)15] = 15;
}
