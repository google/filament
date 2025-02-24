#include "napi.h"

using namespace Napi;

static Value CreateDataView1(const CallbackInfo& info) {
  ArrayBuffer arrayBuffer = info[0].As<ArrayBuffer>();
  return DataView::New(info.Env(), arrayBuffer);
}

static Value CreateDataView2(const CallbackInfo& info) {
  ArrayBuffer arrayBuffer = info[0].As<ArrayBuffer>();
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  return DataView::New(info.Env(), arrayBuffer, byteOffset);
}

static Value CreateDataView3(const CallbackInfo& info) {
  ArrayBuffer arrayBuffer = info[0].As<ArrayBuffer>();
  size_t byteOffset = info[1].As<Number>().Uint32Value();
  size_t byteLength = info[2].As<Number>().Uint32Value();
  return DataView::New(info.Env(), arrayBuffer, byteOffset, byteLength);
}

static Value GetArrayBuffer(const CallbackInfo& info) {
  return info[0].As<DataView>().ArrayBuffer();
}

static Value GetByteOffset(const CallbackInfo& info) {
  return Number::New(info.Env(),
                     static_cast<double>(info[0].As<DataView>().ByteOffset()));
}

static Value GetByteLength(const CallbackInfo& info) {
  return Number::New(info.Env(),
                     static_cast<double>(info[0].As<DataView>().ByteLength()));
}

Object InitDataView(Env env) {
  Object exports = Object::New(env);

  exports["createDataView1"] = Function::New(env, CreateDataView1);
  exports["createDataView2"] = Function::New(env, CreateDataView2);
  exports["createDataView3"] = Function::New(env, CreateDataView3);
  exports["getArrayBuffer"] = Function::New(env, GetArrayBuffer);
  exports["getByteOffset"] = Function::New(env, GetByteOffset);
  exports["getByteLength"] = Function::New(env, GetByteLength);

  return exports;
}
