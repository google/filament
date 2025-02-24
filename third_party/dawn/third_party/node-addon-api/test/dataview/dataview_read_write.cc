#include "napi.h"

using namespace Napi;

static Value GetFloat32(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  return Number::New(info.Env(), info[0].As<DataView>().GetFloat32(byteOffset));
}

static Value GetFloat64(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  return Number::New(info.Env(), info[0].As<DataView>().GetFloat64(byteOffset));
}

static Value GetInt8(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  return Number::New(info.Env(), info[0].As<DataView>().GetInt8(byteOffset));
}

static Value GetInt16(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  return Number::New(info.Env(), info[0].As<DataView>().GetInt16(byteOffset));
}

static Value GetInt32(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  return Number::New(info.Env(), info[0].As<DataView>().GetInt32(byteOffset));
}

static Value GetUint8(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  return Number::New(info.Env(), info[0].As<DataView>().GetUint8(byteOffset));
}

static Value GetUint16(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  return Number::New(info.Env(), info[0].As<DataView>().GetUint16(byteOffset));
}

static Value GetUint32(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  return Number::New(info.Env(), info[0].As<DataView>().GetUint32(byteOffset));
}

static void SetFloat32(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  auto value = info[2].As<Number>().FloatValue();
  info[0].As<DataView>().SetFloat32(byteOffset, value);
}

static void SetFloat64(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  auto value = info[2].As<Number>().DoubleValue();
  info[0].As<DataView>().SetFloat64(byteOffset, value);
}

static void SetInt8(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  auto value = info[2].As<Number>().Int32Value();
  info[0].As<DataView>().SetInt8(byteOffset, value);
}

static void SetInt16(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  auto value = info[2].As<Number>().Int32Value();
  info[0].As<DataView>().SetInt16(byteOffset, value);
}

static void SetInt32(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  auto value = info[2].As<Number>().Int32Value();
  info[0].As<DataView>().SetInt32(byteOffset, value);
}

static void SetUint8(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  auto value = info[2].As<Number>().Uint32Value();
  info[0].As<DataView>().SetUint8(byteOffset, value);
}

static void SetUint16(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  auto value = info[2].As<Number>().Uint32Value();
  info[0].As<DataView>().SetUint16(byteOffset, value);
}

static void SetUint32(const CallbackInfo& info) {
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  auto value = info[2].As<Number>().Uint32Value();
  info[0].As<DataView>().SetUint32(byteOffset, value);
}

Object InitDataViewReadWrite(Env env) {
  Object exports = Object::New(env);

  exports["getFloat32"] = Function::New(env, GetFloat32);
  exports["getFloat64"] = Function::New(env, GetFloat64);
  exports["getInt8"] = Function::New(env, GetInt8);
  exports["getInt16"] = Function::New(env, GetInt16);
  exports["getInt32"] = Function::New(env, GetInt32);
  exports["getUint8"] = Function::New(env, GetUint8);
  exports["getUint16"] = Function::New(env, GetUint16);
  exports["getUint32"] = Function::New(env, GetUint32);

  exports["setFloat32"] = Function::New(env, SetFloat32);
  exports["setFloat64"] = Function::New(env, SetFloat64);
  exports["setInt8"] = Function::New(env, SetInt8);
  exports["setInt16"] = Function::New(env, SetInt16);
  exports["setInt32"] = Function::New(env, SetInt32);
  exports["setUint8"] = Function::New(env, SetUint8);
  exports["setUint16"] = Function::New(env, SetUint16);
  exports["setUint32"] = Function::New(env, SetUint32);

  return exports;
}
