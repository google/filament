#include "assert.h"
#include "napi.h"
#include "test_helper.h"
using namespace Napi;

static Reference<Buffer<uint8_t>> weak;

static void RefMoveAssignTests(const Napi::CallbackInfo& info) {
  Napi::Object obj = Napi::Object::New(info.Env());
  obj.Set("tPro", "tTEST");
  Napi::Reference<Napi::Object> ref = Napi::Reference<Napi::Object>::New(obj);
  ref.SuppressDestruct();

  napi_ref obj_ref = static_cast<napi_ref>(ref);
  Napi::Reference<Napi::Object> existingRef =
      Napi::Reference<Napi::Object>(info.Env(), obj_ref);
  assert(ref == existingRef);
  assert(!(ref != existingRef));

  std::string val =
      MaybeUnwrap(existingRef.Value().Get("tPro")).As<Napi::String>();
  assert(val == "tTEST");
  // ------------------------------------------------------------ //
  Napi::Reference<Napi::Object> copyMoveRef = std::move(existingRef);
  assert(copyMoveRef == ref);

  Napi::Reference<Napi::Object> copyAssignRef;
  copyAssignRef = std::move(copyMoveRef);
  assert(copyAssignRef == ref);
}

static void ReferenceRefTests(const Napi::CallbackInfo& info) {
  Napi::Object obj = Napi::Object::New(info.Env());
  Napi::Reference<Napi::Object> ref = Napi::Reference<Napi::Object>::New(obj);

  assert(ref.Ref() == 1);
  assert(ref.Unref() == 0);
}

static void ReferenceResetTests(const Napi::CallbackInfo& info) {
  Napi::Object obj = Napi::Object::New(info.Env());
  Napi::Reference<Napi::Object> ref = Napi::Reference<Napi::Object>::New(obj);
  assert(!ref.IsEmpty());

  ref.Reset();
  assert(ref.IsEmpty());

  Napi::Object newObject = Napi::Object::New(info.Env());
  newObject.Set("n-api", "node");

  ref.Reset(newObject, 1);
  assert(!ref.IsEmpty());

  std::string val = MaybeUnwrap(ref.Value().Get("n-api")).As<Napi::String>();
  assert(val == "node");
}

void CreateWeakArray(const CallbackInfo& info) {
  weak = Weak(Buffer<uint8_t>::New(info.Env(), 1));
  weak.SuppressDestruct();
}

napi_value AccessWeakArrayEmpty(const CallbackInfo& info) {
  Buffer<uint8_t> value = weak.Value();
  return Napi::Boolean::New(info.Env(), value.IsEmpty());
}

Object InitReference(Env env) {
  Object exports = Object::New(env);

  exports["createWeakArray"] = Function::New(env, CreateWeakArray);
  exports["accessWeakArrayEmpty"] = Function::New(env, AccessWeakArrayEmpty);

  exports["refMoveAssignTest"] = Function::New(env, RefMoveAssignTests);
  exports["referenceRefTest"] = Function::New(env, ReferenceRefTests);
  exports["refResetTest"] = Function::New(env, ReferenceResetTests);
  return exports;
}
