#include "napi.h"

using namespace Napi;

// Wrappers for testing Object::Get() for global Objects
Value GetPropertyWithCppStyleStringAsKey(const CallbackInfo& info);
Value GetPropertyWithCStyleStringAsKey(const CallbackInfo& info);
Value GetPropertyWithInt32AsKey(const CallbackInfo& info);
Value GetPropertyWithNapiValueAsKey(const CallbackInfo& info);
void CreateMockTestObject(const CallbackInfo& info);

// Wrapper for testing Object::Set() for global Objects
void SetPropertyWithCStyleStringAsKey(const CallbackInfo& info);
void SetPropertyWithCppStyleStringAsKey(const CallbackInfo& info);
void SetPropertyWithInt32AsKey(const CallbackInfo& info);
void SetPropertyWithNapiValueAsKey(const CallbackInfo& info);

Value HasPropertyWithCStyleStringAsKey(const CallbackInfo& info);
Value HasPropertyWithCppStyleStringAsKey(const CallbackInfo& info);
Value HasPropertyWithNapiValueAsKey(const CallbackInfo& info);

Value DeletePropertyWithCStyleStringAsKey(const CallbackInfo& info);
Value DeletePropertyWithCppStyleStringAsKey(const CallbackInfo& info);
Value DeletePropertyWithInt32AsKey(const CallbackInfo& info);
Value DeletePropertyWithNapiValueAsKey(const CallbackInfo& info);

Object InitGlobalObject(Env env) {
  Object exports = Object::New(env);
  exports["getPropertyWithInt32"] =
      Function::New(env, GetPropertyWithInt32AsKey);
  exports["getPropertyWithNapiValue"] =
      Function::New(env, GetPropertyWithNapiValueAsKey);
  exports["getPropertyWithCppString"] =
      Function::New(env, GetPropertyWithCppStyleStringAsKey);
  exports["getPropertyWithCString"] =
      Function::New(env, GetPropertyWithCStyleStringAsKey);
  exports["createMockTestObject"] = Function::New(env, CreateMockTestObject);
  exports["setPropertyWithCStyleString"] =
      Function::New(env, SetPropertyWithCStyleStringAsKey);
  exports["setPropertyWithCppStyleString"] =
      Function::New(env, SetPropertyWithCppStyleStringAsKey);
  exports["setPropertyWithNapiValue"] =
      Function::New(env, SetPropertyWithNapiValueAsKey);
  exports["setPropertyWithInt32"] =
      Function::New(env, SetPropertyWithInt32AsKey);
  exports["hasPropertyWithCStyleString"] =
      Function::New(env, HasPropertyWithCStyleStringAsKey);
  exports["hasPropertyWithCppStyleString"] =
      Function::New(env, HasPropertyWithCppStyleStringAsKey);
  exports["hasPropertyWithNapiValue"] =
      Function::New(env, HasPropertyWithNapiValueAsKey);
  exports["deletePropertyWithCStyleString"] =
      Function::New(env, DeletePropertyWithCStyleStringAsKey);
  exports["deletePropertyWithCppStyleString"] =
      Function::New(env, DeletePropertyWithCppStyleStringAsKey);
  exports["deletePropertyWithInt32"] =
      Function::New(env, DeletePropertyWithInt32AsKey);
  exports["deletePropertyWithNapiValue"] =
      Function::New(env, DeletePropertyWithNapiValueAsKey);
  return exports;
}
