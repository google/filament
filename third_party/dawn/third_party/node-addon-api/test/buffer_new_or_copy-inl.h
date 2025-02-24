// Same tests on when NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED is defined or not
// defined.

Value CreateOrCopyExternalBuffer(const CallbackInfo& info) {
  finalizeCount = 0;

  InitData(testData, testLength);
  Buffer<uint16_t> buffer =
      Buffer<uint16_t>::NewOrCopy(info.Env(), testData, testLength);

  if (buffer.Length() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  VerifyData(buffer.Data(), testLength);
  return buffer;
}

Value CreateOrCopyExternalBufferWithFinalize(const CallbackInfo& info) {
  finalizeCount = 0;

  uint16_t* data = new uint16_t[testLength];
  InitData(data, testLength);

  Buffer<uint16_t> buffer = Buffer<uint16_t>::NewOrCopy(
      info.Env(), data, testLength, [](Env /*env*/, uint16_t* finalizeData) {
        delete[] finalizeData;
        finalizeCount++;
      });

  if (buffer.Length() != testLength) {
    Error::New(info.Env(), "Incorrect buffer length.")
        .ThrowAsJavaScriptException();
    return Value();
  }

  VerifyData(buffer.Data(), testLength);
  return buffer;
}

Value CreateOrCopyExternalBufferWithFinalizeHint(const CallbackInfo& info) {
  finalizeCount = 0;

  uint16_t* data = new uint16_t[testLength];
  InitData(data, testLength);

  char* hint = nullptr;
  Buffer<uint16_t> buffer = Buffer<uint16_t>::NewOrCopy(
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

  VerifyData(buffer.Data(), testLength);
  return buffer;
}
