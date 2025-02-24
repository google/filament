#include <napi.h>
#include "test_helper.h"
using namespace Napi;

Symbol CreateNewSymbolWithNoArgs(const Napi::CallbackInfo&) {
  return Napi::Symbol();
}

Symbol CreateNewSymbolWithCppStrDesc(const Napi::CallbackInfo& info) {
  String cppStrKey = info[0].As<String>();
  return Napi::Symbol::New(info.Env(), cppStrKey.Utf8Value());
}

Symbol CreateNewSymbolWithCStrDesc(const Napi::CallbackInfo& info) {
  String cStrKey = info[0].As<String>();
  return Napi::Symbol::New(info.Env(), cStrKey.Utf8Value().c_str());
}

Symbol CreateNewSymbolWithNapiString(const Napi::CallbackInfo& info) {
  String strKey = info[0].As<String>();
  return Napi::Symbol::New(info.Env(), strKey);
}

Symbol GetWellknownSymbol(const Napi::CallbackInfo& info) {
  String registrySymbol = info[0].As<String>();
  return MaybeUnwrap(
      Napi::Symbol::WellKnown(info.Env(), registrySymbol.Utf8Value().c_str()));
}

Symbol FetchSymbolFromGlobalRegistry(const Napi::CallbackInfo& info) {
  String registrySymbol = info[0].As<String>();
  return MaybeUnwrap(Napi::Symbol::For(info.Env(), registrySymbol));
}

Symbol FetchSymbolFromGlobalRegistryWithCppKey(const Napi::CallbackInfo& info) {
  String cppStringKey = info[0].As<String>();
  return MaybeUnwrap(Napi::Symbol::For(info.Env(), cppStringKey.Utf8Value()));
}

Symbol FetchSymbolFromGlobalRegistryWithCKey(const Napi::CallbackInfo& info) {
  String cppStringKey = info[0].As<String>();
  return MaybeUnwrap(
      Napi::Symbol::For(info.Env(), cppStringKey.Utf8Value().c_str()));
}

Symbol TestUndefinedSymbolsCanBeCreated(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return MaybeUnwrap(Napi::Symbol::For(env, env.Undefined()));
}

Symbol TestNullSymbolsCanBeCreated(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return MaybeUnwrap(Napi::Symbol::For(env, env.Null()));
}

Object InitSymbol(Env env) {
  Object exports = Object::New(env);

  exports["createNewSymbolWithNoArgs"] =
      Function::New(env, CreateNewSymbolWithNoArgs);
  exports["createNewSymbolWithCppStr"] =
      Function::New(env, CreateNewSymbolWithCppStrDesc);
  exports["createNewSymbolWithCStr"] =
      Function::New(env, CreateNewSymbolWithCStrDesc);
  exports["createNewSymbolWithNapi"] =
      Function::New(env, CreateNewSymbolWithNapiString);
  exports["getWellKnownSymbol"] = Function::New(env, GetWellknownSymbol);
  exports["getSymbolFromGlobalRegistry"] =
      Function::New(env, FetchSymbolFromGlobalRegistry);
  exports["getSymbolFromGlobalRegistryWithCKey"] =
      Function::New(env, FetchSymbolFromGlobalRegistryWithCKey);
  exports["getSymbolFromGlobalRegistryWithCppKey"] =
      Function::New(env, FetchSymbolFromGlobalRegistryWithCppKey);
  exports["testUndefinedSymbolCanBeCreated"] =
      Function::New(env, TestUndefinedSymbolsCanBeCreated);
  exports["testNullSymbolCanBeCreated"] =
      Function::New(env, TestNullSymbolsCanBeCreated);
  return exports;
}
