/* ObjectReference can be used to create references to Values that
are not Objects by creating a blank Object and setting Values to
it. Subclasses of Objects can only be set using an ObjectReference
by first casting it as an Object. */
#include "assert.h"
#include "napi.h"
#include "test_helper.h"

using namespace Napi;

ObjectReference weak;
ObjectReference persistent;
ObjectReference reference;

ObjectReference casted_weak;
ObjectReference casted_persistent;
ObjectReference casted_reference;

// Set keys can be one of:
// C style string, std::string& utf8, and const char *

// Set values can be one of:
//  Napi::Value
//  napi_value (req static_cast)
// const char* (c style string)
// boolean
// double

enum VAL_TYPES { JS = 0, C_STR, CPP_STR, BOOL, INT, DOUBLE, JS_CAST };

void MoveOperatorsTest(const Napi::CallbackInfo& info) {
  Napi::ObjectReference existingRef;
  Napi::ObjectReference existingRef2;
  Napi::Object testObject = Napi::Object::New(info.Env());
  testObject.Set("testProp", "tProp");

  // ObjectReference(Reference<Object>&& other);
  Napi::Reference<Napi::Object> refObj =
      Napi::Reference<Napi::Object>::New(testObject);
  Napi::ObjectReference objRef = std::move(refObj);
  std::string prop = MaybeUnwrap(objRef.Get("testProp")).As<Napi::String>();
  assert(prop == "tProp");

  // ObjectReference& operator=(Reference<Object>&& other);
  Napi::Reference<Napi::Object> refObj2 =
      Napi::Reference<Napi::Object>::New(testObject);
  existingRef = std::move(refObj2);
  prop = MaybeUnwrap(existingRef.Get("testProp")).As<Napi::String>();
  assert(prop == "tProp");

  // ObjectReference(ObjectReference&& other);
  Napi::ObjectReference objRef3 = std::move(existingRef);
  prop = MaybeUnwrap(objRef3.Get("testProp")).As<Napi::String>();
  assert(prop == "tProp");

  // ObjectReference& operator=(ObjectReference&& other);
  existingRef2 = std::move(objRef3);
  prop = MaybeUnwrap(objRef.Get("testProp")).As<Napi::String>();
  assert(prop == "tProp");
}

void SetObjectWithCStringKey(Napi::ObjectReference& obj,
                             Napi::Value key,
                             Napi::Value val,
                             int valType) {
  std::string c_key = key.As<Napi::String>().Utf8Value();
  switch (valType) {
    case JS:
      obj.Set(c_key.c_str(), val);
      break;

    case JS_CAST:
      obj.Set(c_key.c_str(), static_cast<napi_value>(val));
      break;

    case C_STR: {
      std::string c_val = val.As<Napi::String>().Utf8Value();
      obj.Set(c_key.c_str(), c_val.c_str());
      break;
    }

    case BOOL:
      obj.Set(c_key.c_str(), val.As<Napi::Boolean>().Value());
      break;

    case DOUBLE:
      obj.Set(c_key.c_str(), val.As<Napi::Number>().DoubleValue());
      break;
  }
}

void SetObjectWithCppStringKey(Napi::ObjectReference& obj,
                               Napi::Value key,
                               Napi::Value val,
                               int valType) {
  std::string c_key = key.As<Napi::String>();
  switch (valType) {
    case JS:
      obj.Set(c_key, val);
      break;

    case JS_CAST:
      obj.Set(c_key, static_cast<napi_value>(val));
      break;

    case CPP_STR: {
      std::string c_val = val.As<Napi::String>();
      obj.Set(c_key, c_val);
      break;
    }

    case BOOL:
      obj.Set(c_key, val.As<Napi::Boolean>().Value());
      break;

    case DOUBLE:
      obj.Set(c_key, val.As<Napi::Number>().DoubleValue());
      break;
  }
}

void SetObjectWithIntKey(Napi::ObjectReference& obj,
                         Napi::Value key,
                         Napi::Value val,
                         int valType) {
  uint32_t c_key = key.As<Napi::Number>().Uint32Value();
  switch (valType) {
    case JS:
      obj.Set(c_key, val);
      break;

    case JS_CAST:
      obj.Set(c_key, static_cast<napi_value>(val));
      break;

    case C_STR: {
      std::string c_val = val.As<Napi::String>();
      obj.Set(c_key, c_val.c_str());
      break;
    }

    case CPP_STR: {
      std::string cpp_val = val.As<Napi::String>();
      obj.Set(c_key, cpp_val);
      break;
    }

    case BOOL:
      obj.Set(c_key, val.As<Napi::Boolean>().Value());
      break;

    case DOUBLE:
      obj.Set(c_key, val.As<Napi::Number>().DoubleValue());
      break;
  }
}

void SetObject(const Napi::CallbackInfo& info) {
  Env env = info.Env();
  HandleScope scope(env);

  weak = Weak(Object::New(env));
  weak.SuppressDestruct();

  persistent = Persistent(Object::New(env));
  persistent.SuppressDestruct();

  reference = Reference<Object>::New(Object::New(env), 2);
  reference.SuppressDestruct();

  Napi::Object configObject = info[0].As<Napi::Object>();

  int keyType =
      MaybeUnwrap(configObject.Get("keyType")).As<Napi::Number>().Uint32Value();
  int valType =
      MaybeUnwrap(configObject.Get("valType")).As<Napi::Number>().Uint32Value();
  Napi::Value key = MaybeUnwrap(configObject.Get("key"));
  Napi::Value val = MaybeUnwrap(configObject.Get("val"));

  switch (keyType) {
    case CPP_STR:
      SetObjectWithCppStringKey(weak, key, val, valType);
      SetObjectWithCppStringKey(persistent, key, val, valType);
      SetObjectWithCppStringKey(reference, key, val, valType);
      break;

    case C_STR:
      SetObjectWithCStringKey(weak, key, val, valType);
      SetObjectWithCStringKey(persistent, key, val, valType);
      SetObjectWithCStringKey(reference, key, val, valType);
      break;

    case INT:
      SetObjectWithIntKey(weak, key, val, valType);
      SetObjectWithIntKey(persistent, key, val, valType);
      SetObjectWithIntKey(reference, key, val, valType);

    default:
      break;
  }
}

void SetCastedObjects(const CallbackInfo& info) {
  Env env = info.Env();
  HandleScope scope(env);

  Array ex = Array::New(env);
  ex.Set((uint32_t)0, String::New(env, "hello"));
  ex.Set(1, String::New(env, "world"));
  ex.Set(2, String::New(env, "!"));

  casted_weak = Weak(ex.As<Object>());
  casted_weak.SuppressDestruct();

  casted_persistent = Persistent(ex.As<Object>());
  casted_persistent.SuppressDestruct();

  casted_reference = Reference<Object>::New(ex.As<Object>(), 2);
  casted_reference.SuppressDestruct();
}

// info[0] is a flag to determine if the weak, persistent, or
// multiple reference ObjectReference is being requested.
Value GetFromValue(const CallbackInfo& info) {
  Env env = info.Env();

  if (info[0].As<String>() == String::New(env, "weak")) {
    if (weak.IsEmpty()) {
      return String::New(env, "No Referenced Value");
    } else {
      return weak.Value();
    }
  } else if (info[0].As<String>() == String::New(env, "persistent")) {
    return persistent.Value();
  } else {
    return reference.Value();
  }
}

Value GetHelper(ObjectReference& ref,
                Object& configObject,
                const Napi::Env& env) {
  int keyType =
      MaybeUnwrap(configObject.Get("keyType")).As<Napi::Number>().Uint32Value();
  if (ref.IsEmpty()) {
    return String::New(env, "No referenced Value");
  }

  switch (keyType) {
    case C_STR: {
      std::string c_key =
          MaybeUnwrap(configObject.Get("key")).As<String>().Utf8Value();
      return MaybeUnwrap(ref.Get(c_key.c_str()));
      break;
    }
    case CPP_STR: {
      std::string cpp_key =
          MaybeUnwrap(configObject.Get("key")).As<String>().Utf8Value();
      return MaybeUnwrap(ref.Get(cpp_key));
      break;
    }
    case INT: {
      uint32_t key =
          MaybeUnwrap(configObject.Get("key")).As<Number>().Uint32Value();
      return MaybeUnwrap(ref.Get(key));
      break;
    }

    default:
      return String::New(env, "Error: Reached end of getter");
      break;
  }
}

Value GetFromGetters(const CallbackInfo& info) {
  std::string object_req = info[0].As<String>();
  Object configObject = info[1].As<Object>();
  if (object_req == "weak") {
    return GetHelper(weak, configObject, info.Env());
  } else if (object_req == "persistent") {
    return GetHelper(persistent, configObject, info.Env());
  }

  return GetHelper(reference, configObject, info.Env());
}

// info[0] is a flag to determine if the weak, persistent, or
// multiple reference ObjectReference is being requested.
// info[1] is the key, and it be either a String or a Number.
Value GetFromGetter(const CallbackInfo& info) {
  Env env = info.Env();

  if (info[0].As<String>() == String::New(env, "weak")) {
    if (weak.IsEmpty()) {
      return String::New(env, "No Referenced Value");
    } else {
      if (info[1].IsString()) {
        return MaybeUnwrap(weak.Get(info[1].As<String>().Utf8Value()));
      } else if (info[1].IsNumber()) {
        return MaybeUnwrap(weak.Get(info[1].As<Number>().Uint32Value()));
      }
    }
  } else if (info[0].As<String>() == String::New(env, "persistent")) {
    if (info[1].IsString()) {
      return MaybeUnwrap(persistent.Get(info[1].As<String>().Utf8Value()));
    } else if (info[1].IsNumber()) {
      return MaybeUnwrap(persistent.Get(info[1].As<Number>().Uint32Value()));
    }
  } else {
    if (info[0].IsString()) {
      return MaybeUnwrap(reference.Get(info[0].As<String>().Utf8Value()));
    } else if (info[0].IsNumber()) {
      return MaybeUnwrap(reference.Get(info[0].As<Number>().Uint32Value()));
    }
  }

  return String::New(env, "Error: Reached end of getter");
}

// info[0] is a flag to determine if the weak, persistent, or
// multiple reference ObjectReference is being requested.
Value GetCastedFromValue(const CallbackInfo& info) {
  Env env = info.Env();

  if (info[0].As<String>() == String::New(env, "weak")) {
    if (casted_weak.IsEmpty()) {
      return String::New(env, "No Referenced Value");
    } else {
      return casted_weak.Value();
    }
  } else if (info[0].As<String>() == String::New(env, "persistent")) {
    return casted_persistent.Value();
  } else {
    return casted_reference.Value();
  }
}

// info[0] is a flag to determine if the weak, persistent, or
// multiple reference ObjectReference is being requested.
// info[1] is the key and it must be a Number.
Value GetCastedFromGetter(const CallbackInfo& info) {
  Env env = info.Env();

  if (info[0].As<String>() == String::New(env, "weak")) {
    if (casted_weak.IsEmpty()) {
      return String::New(env, "No Referenced Value");
    } else {
      return MaybeUnwrap(casted_weak.Get(info[1].As<Number>()));
    }
  } else if (info[0].As<String>() == String::New(env, "persistent")) {
    return MaybeUnwrap(casted_persistent.Get(info[1].As<Number>()));
  } else {
    return MaybeUnwrap(casted_reference.Get(info[1].As<Number>()));
  }
}

// info[0] is a flag to determine if the weak, persistent, or
// multiple reference ObjectReference is being requested.
Number UnrefObjects(const CallbackInfo& info) {
  Env env = info.Env();
  uint32_t num;

  if (info[0].As<String>() == String::New(env, "weak")) {
    num = weak.Unref();
  } else if (info[0].As<String>() == String::New(env, "persistent")) {
    num = persistent.Unref();
  } else if (info[0].As<String>() == String::New(env, "references")) {
    num = reference.Unref();
  } else if (info[0].As<String>() == String::New(env, "casted weak")) {
    num = casted_weak.Unref();
  } else if (info[0].As<String>() == String::New(env, "casted persistent")) {
    num = casted_persistent.Unref();
  } else {
    num = casted_reference.Unref();
  }

  return Number::New(env, num);
}

// info[0] is a flag to determine if the weak, persistent, or
// multiple reference ObjectReference is being requested.
Number RefObjects(const CallbackInfo& info) {
  Env env = info.Env();
  uint32_t num;

  if (info[0].As<String>() == String::New(env, "weak")) {
    num = weak.Ref();
  } else if (info[0].As<String>() == String::New(env, "persistent")) {
    num = persistent.Ref();
  } else if (info[0].As<String>() == String::New(env, "references")) {
    num = reference.Ref();
  } else if (info[0].As<String>() == String::New(env, "casted weak")) {
    num = casted_weak.Ref();
  } else if (info[0].As<String>() == String::New(env, "casted persistent")) {
    num = casted_persistent.Ref();
  } else {
    num = casted_reference.Ref();
  }

  return Number::New(env, num);
}

Object InitObjectReference(Env env) {
  Object exports = Object::New(env);

  exports["setCastedObjects"] = Function::New(env, SetCastedObjects);
  exports["setObject"] = Function::New(env, SetObject);
  exports["getCastedFromValue"] = Function::New(env, GetCastedFromValue);
  exports["getFromGetters"] = Function::New(env, GetFromGetters);
  exports["getCastedFromGetter"] = Function::New(env, GetCastedFromGetter);
  exports["getFromValue"] = Function::New(env, GetFromValue);
  exports["unrefObjects"] = Function::New(env, UnrefObjects);
  exports["refObjects"] = Function::New(env, RefObjects);
  exports["moveOpTest"] = Function::New(env, MoveOperatorsTest);

  return exports;
}
