#include "buffer.h"
#include "napi.h"

using namespace Napi;

namespace test_buffer {
uint16_t testData[testLength];
int finalizeCount = 0;
}  // namespace test_buffer

using namespace test_buffer;

namespace {
Value CreateBuffer(const CallbackInfo& info) {
  Buffer<uint16_t> buffer = Buffer<uint16_t>::New(info.Env(), testLength);

  if (buffer.Length() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  InitData(buffer.Data(), testLength);
  return buffer;
}

Value CreateExternalBuffer(const CallbackInfo& info) {
  finalizeCount = 0;

  Buffer<uint16_t> buffer =
      Buffer<uint16_t>::New(info.Env(), testData, testLength);

  if (buffer.Length() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  if (buffer.Data() != testData) {
    Error::New(info.Env(), "Incorrect buffer data.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  InitData(testData, testLength);
  return buffer;
}

Value CreateExternalBufferWithFinalize(const CallbackInfo& info) {
  finalizeCount = 0;

  uint16_t* data = new uint16_t[testLength];

  Buffer<uint16_t> buffer = Buffer<uint16_t>::New(
      info.Env(), data, testLength, [](Env /*env*/, uint16_t* finalizeData) {
        delete[] finalizeData;
        finalizeCount++;
      });

  if (buffer.Length() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  if (buffer.Data() != data) {
    Error::New(info.Env(), "Incorrect buffer data.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  InitData(data, testLength);
  return buffer;
}

Value CreateExternalBufferWithFinalizeHint(const CallbackInfo& info) {
  finalizeCount = 0;

  uint16_t* data = new uint16_t[testLength];

  char* hint = nullptr;
  Buffer<uint16_t> buffer = Buffer<uint16_t>::New(
      info.Env(),
      data,
      testLength,
      [](Env /*env*/, uint16_t* finalizeData, char* /*finalizeHint*/) {
        delete[] finalizeData;
        finalizeCount++;
      },
      hint);

  if (buffer.Length() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  if (buffer.Data() != data) {
    Error::New(info.Env(), "Incorrect buffer data.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  InitData(data, testLength);
  return buffer;
}

Value CreateBufferCopy(const CallbackInfo& info) {
  InitData(testData, testLength);

  Buffer<uint16_t> buffer =
      Buffer<uint16_t>::Copy(info.Env(), testData, testLength);

  if (buffer.Length() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  if (buffer.Data() == testData) {
    Error::New(info.Env(), "Copy should have different memory.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  if (!VerifyData(buffer.Data(), buffer.Length())) {
    Error::New(info.Env(), "Copy data is incorrect.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  return buffer;
}

#include "buffer_new_or_copy-inl.h"

void CheckBuffer(const CallbackInfo& info) {
  if (!info[0].IsBuffer()) {
    Error::New(info.Env(), "A buffer was expected.")
        .ThrowAsJavaScriptException();
    return;
  }

  Buffer<uint16_t> buffer = info[0].As<Buffer<uint16_t>>();

  if (buffer.Length() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return;
  }

  if (!VerifyData(buffer.Data(), testLength)) {
    Error::New(info.Env(), "Incorrect buffer data.")
        .ThrowAsJavaScriptException();
    return;
  }
}

Value GetFinalizeCount(const CallbackInfo& info) {
  return Number::New(info.Env(), finalizeCount);
}

}  // end anonymous namespace

Object InitBuffer(Env env) {
  Object exports = Object::New(env);

  exports["createBuffer"] = Function::New(env, CreateBuffer);
  exports["createExternalBuffer"] = Function::New(env, CreateExternalBuffer);
  exports["createExternalBufferWithFinalize"] =
      Function::New(env, CreateExternalBufferWithFinalize);
  exports["createExternalBufferWithFinalizeHint"] =
      Function::New(env, CreateExternalBufferWithFinalizeHint);
  exports["createOrCopyExternalBuffer"] =
      Function::New(env, CreateOrCopyExternalBuffer);
  exports["createOrCopyExternalBufferWithFinalize"] =
      Function::New(env, CreateOrCopyExternalBufferWithFinalize);
  exports["createOrCopyExternalBufferWithFinalizeHint"] =
      Function::New(env, CreateOrCopyExternalBufferWithFinalizeHint);
  exports["createBufferCopy"] = Function::New(env, CreateBufferCopy);
  exports["checkBuffer"] = Function::New(env, CheckBuffer);
  exports["getFinalizeCount"] = Function::New(env, GetFinalizeCount);

  return exports;
}
