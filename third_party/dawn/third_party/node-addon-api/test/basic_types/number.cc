#include <cfloat>

#include "napi.h"

using namespace Napi;

Value ToInt32(const CallbackInfo& info) {
  return Number::New(info.Env(), info[0].As<Number>().Int32Value());
}

Value ToUint32(const CallbackInfo& info) {
  return Number::New(info.Env(), info[0].As<Number>().Uint32Value());
}

Value ToInt64(const CallbackInfo& info) {
  return Number::New(info.Env(),
                     static_cast<double>(info[0].As<Number>().Int64Value()));
}

Value ToFloat(const CallbackInfo& info) {
  return Number::New(info.Env(), info[0].As<Number>().FloatValue());
}

Value ToDouble(const CallbackInfo& info) {
  return Number::New(info.Env(), info[0].As<Number>().DoubleValue());
}

Value MinFloat(const CallbackInfo& info) {
  return Number::New(info.Env(), FLT_MIN);
}

Value MaxFloat(const CallbackInfo& info) {
  return Number::New(info.Env(), FLT_MAX);
}

Value MinDouble(const CallbackInfo& info) {
  return Number::New(info.Env(), DBL_MIN);
}

Value MaxDouble(const CallbackInfo& info) {
  return Number::New(info.Env(), DBL_MAX);
}

Value OperatorInt32(const CallbackInfo& info) {
  Number number = info[0].As<Number>();
  return Boolean::New(info.Env(),
                      number.Int32Value() == static_cast<int32_t>(number));
}

Value OperatorUint32(const CallbackInfo& info) {
  Number number = info[0].As<Number>();
  return Boolean::New(info.Env(),
                      number.Uint32Value() == static_cast<uint32_t>(number));
}

Value OperatorInt64(const CallbackInfo& info) {
  Number number = info[0].As<Number>();
  return Boolean::New(info.Env(),
                      number.Int64Value() == static_cast<int64_t>(number));
}

Value OperatorFloat(const CallbackInfo& info) {
  Number number = info[0].As<Number>();
  return Boolean::New(info.Env(),
                      number.FloatValue() == static_cast<float>(number));
}

Value OperatorDouble(const CallbackInfo& info) {
  Number number = info[0].As<Number>();
  return Boolean::New(info.Env(),
                      number.DoubleValue() == static_cast<double>(number));
}

Value CreateEmptyNumber(const CallbackInfo& info) {
  Number number;
  return Boolean::New(info.Env(), number.IsEmpty());
}

Value CreateNumberFromExistingValue(const CallbackInfo& info) {
  return info[0].As<Number>();
}

Object InitBasicTypesNumber(Env env) {
  Object exports = Object::New(env);

  exports["toInt32"] = Function::New(env, ToInt32);
  exports["toUint32"] = Function::New(env, ToUint32);
  exports["toInt64"] = Function::New(env, ToInt64);
  exports["toFloat"] = Function::New(env, ToFloat);
  exports["toDouble"] = Function::New(env, ToDouble);
  exports["minFloat"] = Function::New(env, MinFloat);
  exports["maxFloat"] = Function::New(env, MaxFloat);
  exports["minDouble"] = Function::New(env, MinDouble);
  exports["maxDouble"] = Function::New(env, MaxDouble);
  exports["operatorInt32"] = Function::New(env, OperatorInt32);
  exports["operatorUint32"] = Function::New(env, OperatorUint32);
  exports["operatorInt64"] = Function::New(env, OperatorInt64);
  exports["operatorFloat"] = Function::New(env, OperatorFloat);
  exports["operatorDouble"] = Function::New(env, OperatorDouble);
  exports["createEmptyNumber"] = Function::New(env, CreateEmptyNumber);
  exports["createNumberFromExistingValue"] =
      Function::New(env, CreateNumberFromExistingValue);

  return exports;
}
