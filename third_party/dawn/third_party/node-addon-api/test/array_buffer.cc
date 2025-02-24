#include "napi.h"

using namespace Napi;

namespace {

const size_t testLength = 4;
uint8_t testData[testLength];
int finalizeCount = 0;

void InitData(uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    data[i] = static_cast<uint8_t>(i);
  }
}

bool VerifyData(uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    if (data[i] != static_cast<uint8_t>(i)) {
      return false;
    }
  }
  return true;
}

Value CreateBuffer(const CallbackInfo& info) {
  ArrayBuffer buffer = ArrayBuffer::New(info.Env(), testLength);

  if (buffer.ByteLength() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  InitData(static_cast<uint8_t*>(buffer.Data()), testLength);
  return buffer;
}

Value CreateExternalBuffer(const CallbackInfo& info) {
  finalizeCount = 0;

  ArrayBuffer buffer = ArrayBuffer::New(info.Env(), testData, testLength);

  if (buffer.ByteLength() != testLength) {
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

  uint8_t* data = new uint8_t[testLength];

  ArrayBuffer buffer = ArrayBuffer::New(
      info.Env(), data, testLength, [](Env /*env*/, void* finalizeData) {
        delete[] static_cast<uint8_t*>(finalizeData);
        finalizeCount++;
      });

  if (buffer.ByteLength() != testLength) {
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

  uint8_t* data = new uint8_t[testLength];

  char* hint = nullptr;
  ArrayBuffer buffer = ArrayBuffer::New(
      info.Env(),
      data,
      testLength,
      [](Env /*env*/, void* finalizeData, char* /*finalizeHint*/) {
        delete[] static_cast<uint8_t*>(finalizeData);
        finalizeCount++;
      },
      hint);

  if (buffer.ByteLength() != testLength) {
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

void CheckBuffer(const CallbackInfo& info) {
  if (!info[0].IsArrayBuffer()) {
    Error::New(info.Env(), "A buffer was expected.")
        .ThrowAsJavaScriptException();
    return;
  }

  ArrayBuffer buffer = info[0].As<ArrayBuffer>();

  if (buffer.ByteLength() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return;
  }

  if (!VerifyData(static_cast<uint8_t*>(buffer.Data()), testLength)) {
    Error::New(info.Env(), "Incorrect buffer data.")
        .ThrowAsJavaScriptException();
    return;
  }
}

Value GetFinalizeCount(const CallbackInfo& info) {
  return Number::New(info.Env(), finalizeCount);
}

Value CreateBufferWithConstructor(const CallbackInfo& info) {
  ArrayBuffer buffer = ArrayBuffer::New(info.Env(), testLength);
  if (buffer.ByteLength() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return Value();
  }
  InitData(static_cast<uint8_t*>(buffer.Data()), testLength);
  ArrayBuffer buffer2(info.Env(), buffer);
  return buffer2;
}

Value CheckEmptyBuffer(const CallbackInfo& info) {
  ArrayBuffer buffer;
  return Boolean::New(info.Env(), buffer.IsEmpty());
}

void CheckDetachUpdatesData(const CallbackInfo& info) {
  if (!info[0].IsArrayBuffer()) {
    Error::New(info.Env(), "A buffer was expected.")
        .ThrowAsJavaScriptException();
    return;
  }

  ArrayBuffer buffer = info[0].As<ArrayBuffer>();

  // This potentially causes the buffer to cache its data pointer and length.
  buffer.Data();
  buffer.ByteLength();

#if NAPI_VERSION >= 7
  if (buffer.IsDetached()) {
    Error::New(info.Env(), "Buffer should not be detached.")
        .ThrowAsJavaScriptException();
    return;
  }
#endif

  if (info.Length() == 2) {
    // Detach externally (in JavaScript).
    if (!info[1].IsFunction()) {
      Error::New(info.Env(), "A function was expected.")
          .ThrowAsJavaScriptException();
      return;
    }

    Function detach = info[1].As<Function>();
    detach.Call({});
  } else {
#if NAPI_VERSION >= 7
    // Detach directly.
    buffer.Detach();
#else
    return;
#endif
  }

#if NAPI_VERSION >= 7
  if (!buffer.IsDetached()) {
    Error::New(info.Env(), "Buffer should be detached.")
        .ThrowAsJavaScriptException();
    return;
  }
#endif

  if (buffer.Data() != nullptr) {
    Error::New(info.Env(), "Incorrect data pointer.")
        .ThrowAsJavaScriptException();
    return;
  }

  if (buffer.ByteLength() != 0) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return;
  }
}

}  // end anonymous namespace

Object InitArrayBuffer(Env env) {
  Object exports = Object::New(env);

  exports["createBuffer"] = Function::New(env, CreateBuffer);
  exports["createExternalBuffer"] = Function::New(env, CreateExternalBuffer);
  exports["createExternalBufferWithFinalize"] =
      Function::New(env, CreateExternalBufferWithFinalize);
  exports["createExternalBufferWithFinalizeHint"] =
      Function::New(env, CreateExternalBufferWithFinalizeHint);
  exports["checkBuffer"] = Function::New(env, CheckBuffer);
  exports["getFinalizeCount"] = Function::New(env, GetFinalizeCount);
  exports["createBufferWithConstructor"] =
      Function::New(env, CreateBufferWithConstructor);
  exports["checkEmptyBuffer"] = Function::New(env, CheckEmptyBuffer);
  exports["checkDetachUpdatesData"] =
      Function::New(env, CheckDetachUpdatesData);

  return exports;
}
