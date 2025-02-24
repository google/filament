#define NODE_API_NO_EXTERNAL_BUFFERS_ALLOWED
// Should compile without errors
#include "buffer.h"
#include "napi.h"

using namespace Napi;
using namespace test_buffer;

namespace {
#include "buffer_new_or_copy-inl.h"
}

Object InitBufferNoExternal(Env env) {
  Object exports = Object::New(env);

  exports["createOrCopyExternalBuffer"] =
      Function::New(env, CreateOrCopyExternalBuffer);
  exports["createOrCopyExternalBufferWithFinalize"] =
      Function::New(env, CreateOrCopyExternalBufferWithFinalize);
  exports["createOrCopyExternalBufferWithFinalizeHint"] =
      Function::New(env, CreateOrCopyExternalBufferWithFinalizeHint);

  return exports;
}
