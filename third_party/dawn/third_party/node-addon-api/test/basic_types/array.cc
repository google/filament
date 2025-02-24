#include "napi.h"

using namespace Napi;

Value CreateArray(const CallbackInfo& info) {
  if (info.Length() > 0) {
    size_t length = info[0].As<Number>().Uint32Value();
    return Array::New(info.Env(), length);
  } else {
    return Array::New(info.Env());
  }
}

Value GetLength(const CallbackInfo& info) {
  Array array = info[0].As<Array>();
  return Number::New(info.Env(), static_cast<uint32_t>(array.Length()));
}

Value GetElement(const CallbackInfo& info) {
  Array array = info[0].As<Array>();
  size_t index = info[1].As<Number>().Uint32Value();
  return array[index];
}

void SetElement(const CallbackInfo& info) {
  Array array = info[0].As<Array>();
  size_t index = info[1].As<Number>().Uint32Value();
  array[index] = info[2].As<Value>();
}

Object InitBasicTypesArray(Env env) {
  Object exports = Object::New(env);

  exports["createArray"] = Function::New(env, CreateArray);
  exports["getLength"] = Function::New(env, GetLength);
  exports["get"] = Function::New(env, GetElement);
  exports["set"] = Function::New(env, SetElement);

  return exports;
}
