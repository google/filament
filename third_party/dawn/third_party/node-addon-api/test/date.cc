#include "napi.h"

using namespace Napi;

#if (NAPI_VERSION > 4)
namespace {

Value CreateDate(const CallbackInfo& info) {
  double input = info[0].As<Number>().DoubleValue();

  return Date::New(info.Env(), input);
}

Value IsDate(const CallbackInfo& info) {
  Date input = info[0].As<Date>();

  return Boolean::New(info.Env(), input.IsDate());
}

Value ValueOf(const CallbackInfo& info) {
  Date input = info[0].As<Date>();

  return Number::New(info.Env(), input.ValueOf());
}

Value OperatorValue(const CallbackInfo& info) {
  Date input = info[0].As<Date>();

  return Boolean::New(info.Env(),
                      input.ValueOf() == static_cast<double>(input));
}

}  // anonymous namespace

Object InitDate(Env env) {
  Object exports = Object::New(env);
  exports["CreateDate"] = Function::New(env, CreateDate);
  exports["IsDate"] = Function::New(env, IsDate);
  exports["ValueOf"] = Function::New(env, ValueOf);
  exports["OperatorValue"] = Function::New(env, OperatorValue);

  return exports;
}

#endif
