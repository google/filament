#include "napi.h"

using namespace Napi;

namespace {

int testData = 1;
int finalizeCount = 0;

Value CreateExternal(const CallbackInfo& info) {
  finalizeCount = 0;
  return External<int>::New(info.Env(), &testData);
}

Value CreateExternalWithFinalize(const CallbackInfo& info) {
  finalizeCount = 0;
  return External<int>::New(info.Env(), new int(1), [](Env /*env*/, int* data) {
    delete data;
    finalizeCount++;
  });
}

Value CreateExternalWithFinalizeHint(const CallbackInfo& info) {
  finalizeCount = 0;
  char* hint = nullptr;
  return External<int>::New(
      info.Env(),
      new int(1),
      [](Env /*env*/, int* data, char* /*hint*/) {
        delete data;
        finalizeCount++;
      },
      hint);
}

void CheckExternal(const CallbackInfo& info) {
  Value arg = info[0];
  if (arg.Type() != napi_external) {
    Error::New(info.Env(), "An external argument was expected.")
        .ThrowAsJavaScriptException();
    return;
  }

  External<int> external = arg.As<External<int>>();
  int* externalData = external.Data();
  if (externalData == nullptr || *externalData != 1) {
    Error::New(info.Env(), "An external value of 1 was expected.")
        .ThrowAsJavaScriptException();
    return;
  }
}

Value GetFinalizeCount(const CallbackInfo& info) {
  return Number::New(info.Env(), finalizeCount);
}

Value CreateExternalWithFinalizeException(const CallbackInfo& info) {
  return External<int>::New(info.Env(), new int(1), [](Env env, int* data) {
    Error error = Error::New(env, "Finalizer exception");
    delete data;
#ifdef NAPI_CPP_EXCEPTIONS
    throw error;
#else
      error.ThrowAsJavaScriptException();
#endif
  });
}

}  // end anonymous namespace

Object InitExternal(Env env) {
  Object exports = Object::New(env);

  exports["createExternal"] = Function::New(env, CreateExternal);
  exports["createExternalWithFinalize"] =
      Function::New(env, CreateExternalWithFinalize);
  exports["createExternalWithFinalizeException"] =
      Function::New(env, CreateExternalWithFinalizeException);
  exports["createExternalWithFinalizeHint"] =
      Function::New(env, CreateExternalWithFinalizeHint);
  exports["checkExternal"] = Function::New(env, CheckExternal);
  exports["getFinalizeCount"] = Function::New(env, GetFinalizeCount);

  return exports;
}
