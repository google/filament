#include <assert.h>
#include "napi.h"
using namespace Napi;

struct TestCBInfoSetData {
  static void Test(napi_env env, napi_callback_info info) {
    Napi::CallbackInfo cbInfo(env, info);
    int valuePointer = 1220202;
    cbInfo.SetData(&valuePointer);

    int* placeHolder = static_cast<int*>(cbInfo.Data());
    assert(*(placeHolder) == valuePointer);
    assert(placeHolder == &valuePointer);
  }
};

void TestCallbackInfoSetData(const Napi::CallbackInfo& info) {
  napi_callback_info cb_info = static_cast<napi_callback_info>(info);
  TestCBInfoSetData::Test(info.Env(), cb_info);
}

Object InitCallbackInfo(Env env) {
  Object exports = Object::New(env);

  exports["testCbSetData"] = Function::New(env, TestCallbackInfoSetData);
  return exports;
}
