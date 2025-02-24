#include "napi.h"
#include "test_helper.h"

using namespace Napi;

class FuncRefObject : public Napi::ObjectWrap<FuncRefObject> {
 public:
  FuncRefObject(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<FuncRefObject>(info) {
    Napi::Env env = info.Env();
    int argLen = info.Length();
    if (argLen <= 0 || !info[0].IsNumber()) {
      Napi::TypeError::New(env, "First param should be a number")
          .ThrowAsJavaScriptException();
      return;
    }
    Napi::Number value = info[0].As<Napi::Number>();
    this->_value = value.Int32Value();
  }

  Napi::Value GetValue(const Napi::CallbackInfo& info) {
    int value = this->_value;
    return Napi::Number::New(info.Env(), value);
  }

 private:
  int _value;
};

namespace {

Value ConstructRefFromExisitingRef(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  FunctionReference ref;
  FunctionReference movedRef;
  ref.Reset(info[0].As<Function>());
  movedRef = std::move(ref);

  return MaybeUnwrap(movedRef({}));
}

Value CallWithVectorArgs(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  std::vector<napi_value> newVec;
  FunctionReference ref;
  ref.Reset(info[0].As<Function>());

  for (int i = 1; i < (int)info.Length(); i++) {
    newVec.push_back(info[i]);
  }
  return MaybeUnwrap(ref.Call(newVec));
}

Value CallWithInitList(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  FunctionReference ref;
  ref.Reset(info[0].As<Function>());

  return MaybeUnwrap(ref.Call({info[1], info[2], info[3]}));
}

Value CallWithRecvInitList(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  FunctionReference ref;
  ref.Reset(info[0].As<Function>());

  return MaybeUnwrap(ref.Call(info[1], {info[2], info[3], info[4]}));
}

Value CallWithRecvVector(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  FunctionReference ref;
  std::vector<napi_value> newVec;
  ref.Reset(info[0].As<Function>());

  for (int i = 2; i < (int)info.Length(); i++) {
    newVec.push_back(info[i]);
  }
  return MaybeUnwrap(ref.Call(info[1], newVec));
}

Value CallWithRecvArgc(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  FunctionReference ref;
  ref.Reset(info[0].As<Function>());

  size_t argLength = info.Length() > 2 ? info.Length() - 2 : 0;
  std::unique_ptr<napi_value[]> args{argLength > 0 ? new napi_value[argLength]
                                                   : nullptr};
  for (size_t i = 0; i < argLength; ++i) {
    args[i] = info[i + 2];
  }

  return MaybeUnwrap(ref.Call(info[1], argLength, args.get()));
}

Value MakeAsyncCallbackWithInitList(const Napi::CallbackInfo& info) {
  Napi::FunctionReference ref;
  ref.Reset(info[0].As<Function>());

  Napi::AsyncContext context(info.Env(), "func_ref_resources", {});

  return MaybeUnwrap(
      ref.MakeCallback(Napi::Object::New(info.Env()), {}, context));
}

Value MakeAsyncCallbackWithVector(const Napi::CallbackInfo& info) {
  Napi::FunctionReference ref;
  ref.Reset(info[0].As<Function>());
  std::vector<napi_value> newVec;
  Napi::AsyncContext context(info.Env(), "func_ref_resources", {});

  for (int i = 1; i < (int)info.Length(); i++) {
    newVec.push_back(info[i]);
  }

  return MaybeUnwrap(
      ref.MakeCallback(Napi::Object::New(info.Env()), newVec, context));
}

Value MakeAsyncCallbackWithArgv(const Napi::CallbackInfo& info) {
  Napi::FunctionReference ref;
  ref.Reset(info[0].As<Function>());

  size_t argLength = info.Length() > 1 ? info.Length() - 1 : 0;
  std::unique_ptr<napi_value[]> args{argLength > 0 ? new napi_value[argLength]
                                                   : nullptr};
  for (size_t i = 0; i < argLength; ++i) {
    args[i] = info[i + 1];
  }

  Napi::AsyncContext context(info.Env(), "func_ref_resources", {});
  return MaybeUnwrap(ref.MakeCallback(Napi::Object::New(info.Env()),
                                      argLength,
                                      argLength > 0 ? args.get() : nullptr,
                                      context));
}

Value CreateFunctionReferenceUsingNew(const Napi::CallbackInfo& info) {
  Napi::Function func = ObjectWrap<FuncRefObject>::DefineClass(
      info.Env(),
      "MyObject",
      {ObjectWrap<FuncRefObject>::InstanceMethod("getValue",
                                                 &FuncRefObject::GetValue)});
  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);

  return MaybeUnwrapOr(constructor->New({info[0].As<Number>()}), Object());
}

Value CreateFunctionReferenceUsingNewVec(const Napi::CallbackInfo& info) {
  Napi::Function func = ObjectWrap<FuncRefObject>::DefineClass(
      info.Env(),
      "MyObject",
      {ObjectWrap<FuncRefObject>::InstanceMethod("getValue",
                                                 &FuncRefObject::GetValue)});
  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  std::vector<napi_value> newVec;
  newVec.push_back(info[0]);

  return MaybeUnwrapOr(constructor->New(newVec), Object());
}

Value Call(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  FunctionReference ref;
  ref.Reset(info[0].As<Function>());

  return MaybeUnwrapOr(ref.Call({}), Value());
}

Value Construct(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  FunctionReference ref;
  ref.Reset(info[0].As<Function>());

  return MaybeUnwrapOr(ref.New({}), Object());
}
}  // namespace

Object InitFunctionReference(Env env) {
  Object exports = Object::New(env);
  exports["CreateFuncRefWithNew"] =
      Function::New(env, CreateFunctionReferenceUsingNew);
  exports["CreateFuncRefWithNewVec"] =
      Function::New(env, CreateFunctionReferenceUsingNewVec);
  exports["CallWithRecvArgc"] = Function::New(env, CallWithRecvArgc);
  exports["CallWithRecvVector"] = Function::New(env, CallWithRecvVector);
  exports["CallWithRecvInitList"] = Function::New(env, CallWithRecvInitList);
  exports["CallWithInitList"] = Function::New(env, CallWithInitList);
  exports["CallWithVec"] = Function::New(env, CallWithVectorArgs);
  exports["ConstructWithMove"] =
      Function::New(env, ConstructRefFromExisitingRef);
  exports["AsyncCallWithInitList"] =
      Function::New(env, MakeAsyncCallbackWithInitList);
  exports["AsyncCallWithVector"] =
      Function::New(env, MakeAsyncCallbackWithVector);
  exports["AsyncCallWithArgv"] = Function::New(env, MakeAsyncCallbackWithArgv);
  exports["call"] = Function::New(env, Call);
  exports["construct"] = Function::New(env, Construct);

  return exports;
}
