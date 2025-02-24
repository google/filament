#include <memory>
#include "napi.h"
#include "test_helper.h"

using namespace Napi;

namespace {

int testData = 1;

Boolean EmptyConstructor(const CallbackInfo& info) {
  auto env = info.Env();
  bool isEmpty = info[0].As<Boolean>();
  Function function = isEmpty ? Function() : Function(env, Object::New(env));
  return Boolean::New(env, function.IsEmpty());
}

void VoidCallback(const CallbackInfo& info) {
  auto env = info.Env();
  Object obj = info[0].As<Object>();

  obj["foo"] = String::New(env, "bar");
}

Value ValueCallback(const CallbackInfo& info) {
  auto env = info.Env();
  Object obj = Object::New(env);

  obj["foo"] = String::New(env, "bar");

  return obj;
}

void VoidCallbackWithData(const CallbackInfo& info) {
  auto env = info.Env();
  Object obj = info[0].As<Object>();

  obj["foo"] = String::New(env, "bar");

  int* data = static_cast<int*>(info.Data());
  obj["data"] = Number::New(env, *data);
}

Value ValueCallbackWithData(const CallbackInfo& info) {
  auto env = info.Env();
  Object obj = Object::New(env);

  obj["foo"] = String::New(env, "bar");

  int* data = static_cast<int*>(info.Data());
  obj["data"] = Number::New(env, *data);

  return obj;
}

Value CallWithArgs(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  return MaybeUnwrap(
      func.Call(std::initializer_list<napi_value>{info[1], info[2], info[3]}));
}

Value CallWithVector(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  std::vector<napi_value> args;
  args.reserve(3);
  args.push_back(info[1]);
  args.push_back(info[2]);
  args.push_back(info[3]);
  return MaybeUnwrap(func.Call(args));
}

Value CallWithVectorUsingCppWrapper(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  std::vector<Value> args;
  args.reserve(3);
  args.push_back(info[1]);
  args.push_back(info[2]);
  args.push_back(info[3]);
  return MaybeUnwrap(func.Call(args));
}

Value CallWithCStyleArray(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  std::vector<napi_value> args;
  args.reserve(3);
  args.push_back(info[1]);
  args.push_back(info[2]);
  args.push_back(info[3]);
  return MaybeUnwrap(func.Call(args.size(), args.data()));
}

Value CallWithReceiverAndCStyleArray(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  Value receiver = info[1];
  std::vector<napi_value> args;
  args.reserve(3);
  args.push_back(info[2]);
  args.push_back(info[3]);
  args.push_back(info[4]);
  return MaybeUnwrap(func.Call(receiver, args.size(), args.data()));
}

Value CallWithReceiverAndArgs(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  Value receiver = info[1];
  return MaybeUnwrap(func.Call(
      receiver, std::initializer_list<napi_value>{info[2], info[3], info[4]}));
}

Value CallWithReceiverAndVector(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  Value receiver = info[1];
  std::vector<napi_value> args;
  args.reserve(3);
  args.push_back(info[2]);
  args.push_back(info[3]);
  args.push_back(info[4]);
  return MaybeUnwrap(func.Call(receiver, args));
}

Value CallWithReceiverAndVectorUsingCppWrapper(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  Value receiver = info[1];
  std::vector<Value> args;
  args.reserve(3);
  args.push_back(info[2]);
  args.push_back(info[3]);
  args.push_back(info[4]);
  return MaybeUnwrap(func.Call(receiver, args));
}

Value CallWithInvalidReceiver(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  return MaybeUnwrapOr(func.Call(Value(), std::initializer_list<napi_value>{}),
                       Value());
}

Value CallConstructorWithArgs(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  return MaybeUnwrap(
      func.New(std::initializer_list<napi_value>{info[1], info[2], info[3]}));
}

Value CallConstructorWithVector(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  std::vector<napi_value> args;
  args.reserve(3);
  args.push_back(info[1]);
  args.push_back(info[2]);
  args.push_back(info[3]);
  return MaybeUnwrap(func.New(args));
}

Value CallConstructorWithCStyleArray(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  std::vector<napi_value> args;
  args.reserve(3);
  args.push_back(info[1]);
  args.push_back(info[2]);
  args.push_back(info[3]);
  return MaybeUnwrap(func.New(args.size(), args.data()));
}

void IsConstructCall(const CallbackInfo& info) {
  Function callback = info[0].As<Function>();
  bool isConstructCall = info.IsConstructCall();
  callback({Napi::Boolean::New(info.Env(), isConstructCall)});
}

Value NewTargetCallback(const CallbackInfo& info) {
  return info.NewTarget();
}

void MakeCallbackWithArgs(const CallbackInfo& info) {
  Env env = info.Env();
  Function callback = info[0].As<Function>();
  Object resource = info[1].As<Object>();

  AsyncContext context(env, "function_test_context", resource);

  callback.MakeCallback(
      resource,
      std::initializer_list<napi_value>{info[2], info[3], info[4]},
      context);
}

void MakeCallbackWithVector(const CallbackInfo& info) {
  Env env = info.Env();
  Function callback = info[0].As<Function>();
  Object resource = info[1].As<Object>();

  AsyncContext context(env, "function_test_context", resource);

  std::vector<napi_value> args;
  args.reserve(3);
  args.push_back(info[2]);
  args.push_back(info[3]);
  args.push_back(info[4]);
  callback.MakeCallback(resource, args, context);
}

void MakeCallbackWithCStyleArray(const CallbackInfo& info) {
  Env env = info.Env();
  Function callback = info[0].As<Function>();
  Object resource = info[1].As<Object>();

  AsyncContext context(env, "function_test_context", resource);

  std::vector<napi_value> args;
  args.reserve(3);
  args.push_back(info[2]);
  args.push_back(info[3]);
  args.push_back(info[4]);
  callback.MakeCallback(resource, args.size(), args.data(), context);
}

void MakeCallbackWithInvalidReceiver(const CallbackInfo& info) {
  Function callback = info[0].As<Function>();
  callback.MakeCallback(Value(), std::initializer_list<napi_value>{});
}

Value CallWithFunctionOperator(const CallbackInfo& info) {
  Function func = info[0].As<Function>();
  return MaybeUnwrap(func({info[1], info[2], info[3]}));
}

}  // end anonymous namespace

Object InitFunction(Env env) {
  Object result = Object::New(env);
  Object exports = Object::New(env);
  exports["emptyConstructor"] = Function::New(env, EmptyConstructor);
  exports["voidCallback"] = Function::New(env, VoidCallback, "voidCallback");
  exports["valueCallback"] =
      Function::New(env, ValueCallback, std::string("valueCallback"));
  exports["voidCallbackWithData"] =
      Function::New(env, VoidCallbackWithData, nullptr, &testData);
  exports["valueCallbackWithData"] =
      Function::New(env, ValueCallbackWithData, nullptr, &testData);
  exports["newTargetCallback"] =
      Function::New(env, NewTargetCallback, std::string("newTargetCallback"));
  exports["callWithArgs"] = Function::New(env, CallWithArgs);
  exports["callWithVector"] = Function::New(env, CallWithVector);
  exports["callWithVectorUsingCppWrapper"] =
      Function::New(env, CallWithVectorUsingCppWrapper);
  exports["callWithCStyleArray"] = Function::New(env, CallWithCStyleArray);
  exports["callWithReceiverAndCStyleArray"] =
      Function::New(env, CallWithReceiverAndCStyleArray);
  exports["callWithReceiverAndArgs"] =
      Function::New(env, CallWithReceiverAndArgs);
  exports["callWithReceiverAndVector"] =
      Function::New(env, CallWithReceiverAndVector);
  exports["callWithReceiverAndVectorUsingCppWrapper"] =
      Function::New(env, CallWithReceiverAndVectorUsingCppWrapper);
  exports["callWithInvalidReceiver"] =
      Function::New(env, CallWithInvalidReceiver);
  exports["callConstructorWithArgs"] =
      Function::New(env, CallConstructorWithArgs);
  exports["callConstructorWithVector"] =
      Function::New(env, CallConstructorWithVector);
  exports["callConstructorWithCStyleArray"] =
      Function::New(env, CallConstructorWithCStyleArray);
  exports["isConstructCall"] = Function::New(env, IsConstructCall);
  exports["makeCallbackWithArgs"] = Function::New(env, MakeCallbackWithArgs);
  exports["makeCallbackWithVector"] =
      Function::New(env, MakeCallbackWithVector);
  exports["makeCallbackWithCStyleArray"] =
      Function::New(env, MakeCallbackWithCStyleArray);
  exports["makeCallbackWithInvalidReceiver"] =
      Function::New(env, MakeCallbackWithInvalidReceiver);
  exports["callWithFunctionOperator"] =
      Function::New(env, CallWithFunctionOperator);
  result["plain"] = exports;

  exports = Object::New(env);
  exports["emptyConstructor"] = Function::New(env, EmptyConstructor);
  exports["voidCallback"] = Function::New<VoidCallback>(env, "voidCallback");
  exports["valueCallback"] =
      Function::New<ValueCallback>(env, std::string("valueCallback"));
  exports["newTargetCallback"] =
      Function::New<NewTargetCallback>(env, std::string("newTargetCallback"));
  exports["voidCallbackWithData"] =
      Function::New<VoidCallbackWithData>(env, nullptr, &testData);
  exports["valueCallbackWithData"] =
      Function::New<ValueCallbackWithData>(env, nullptr, &testData);
  exports["callWithArgs"] = Function::New<CallWithArgs>(env);
  exports["callWithVector"] = Function::New<CallWithVector>(env);
  exports["callWithVectorUsingCppWrapper"] =
      Function::New<CallWithVectorUsingCppWrapper>(env);
  exports["callWithCStyleArray"] = Function::New<CallWithCStyleArray>(env);
  exports["callWithReceiverAndCStyleArray"] =
      Function::New<CallWithReceiverAndCStyleArray>(env);
  exports["callWithReceiverAndArgs"] =
      Function::New<CallWithReceiverAndArgs>(env);
  exports["callWithReceiverAndVector"] =
      Function::New<CallWithReceiverAndVector>(env);
  exports["callWithReceiverAndVectorUsingCppWrapper"] =
      Function::New<CallWithReceiverAndVectorUsingCppWrapper>(env);
  exports["callWithInvalidReceiver"] =
      Function::New<CallWithInvalidReceiver>(env);
  exports["callConstructorWithArgs"] =
      Function::New<CallConstructorWithArgs>(env);
  exports["callConstructorWithVector"] =
      Function::New<CallConstructorWithVector>(env);
  exports["callConstructorWithCStyleArray"] =
      Function::New<CallConstructorWithCStyleArray>(env);
  exports["isConstructCall"] = Function::New<IsConstructCall>(env);
  exports["makeCallbackWithArgs"] = Function::New<MakeCallbackWithArgs>(env);
  exports["makeCallbackWithVector"] =
      Function::New<MakeCallbackWithVector>(env);
  exports["makeCallbackWithCStyleArray"] =
      Function::New<MakeCallbackWithCStyleArray>(env);
  exports["makeCallbackWithInvalidReceiver"] =
      Function::New<MakeCallbackWithInvalidReceiver>(env);
  exports["callWithFunctionOperator"] =
      Function::New<CallWithFunctionOperator>(env);
  result["templated"] = exports;

  exports = Object::New(env);
  exports["lambdaWithNoCapture"] =
      Function::New(env, [](const CallbackInfo& info) {
        auto env = info.Env();
        return Boolean::New(env, true);
      });
  exports["lambdaWithCapture"] =
      Function::New(env, [data = 42](const CallbackInfo& info) {
        auto env = info.Env();
        return Boolean::New(env, data == 42);
      });
  exports["lambdaWithMoveOnlyCapture"] = Function::New(
      env, [data = std::make_unique<int>(42)](const CallbackInfo& info) {
        auto env = info.Env();
        return Boolean::New(env, *data == 42);
      });
  result["lambda"] = exports;

  return result;
}
