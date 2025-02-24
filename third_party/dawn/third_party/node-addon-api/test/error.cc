#include <string.h>
#include <future>
#include "assert.h"
#include "napi.h"

using namespace Napi;

namespace {

std::promise<void> promise_for_child_process_;
std::promise<void> promise_for_worker_thread_;

void ResetPromises(const CallbackInfo&) {
  promise_for_child_process_ = std::promise<void>();
  promise_for_worker_thread_ = std::promise<void>();
}

void WaitForWorkerThread(const CallbackInfo&) {
  std::future<void> future = promise_for_worker_thread_.get_future();

  std::future_status status = future.wait_for(std::chrono::seconds(5));

  if (status != std::future_status::ready) {
    Error::Fatal("WaitForWorkerThread", "status != std::future_status::ready");
  }
}

void ReleaseAndWaitForChildProcess(const CallbackInfo& info,
                                   const uint32_t index) {
  if (info.Length() < index + 1) {
    return;
  }

  if (!info[index].As<Boolean>().Value()) {
    return;
  }

  promise_for_worker_thread_.set_value();

  std::future<void> future = promise_for_child_process_.get_future();

  std::future_status status = future.wait_for(std::chrono::seconds(5));

  if (status != std::future_status::ready) {
    Error::Fatal("ReleaseAndWaitForChildProcess",
                 "status != std::future_status::ready");
  }
}

void ReleaseWorkerThread(const CallbackInfo&) {
  promise_for_child_process_.set_value();
}

void DoNotCatch(const CallbackInfo& info) {
  Function thrower = info[0].As<Function>();
  thrower({});
}

void ThrowApiError(const CallbackInfo& info) {
  // Attempting to call an empty function value will throw an API error.
  Function(info.Env(), nullptr).Call(std::initializer_list<napi_value>{});
}

void LastExceptionErrorCode(const CallbackInfo& info) {
  // Previously, `napi_extended_error_info.error_code` got reset to `napi_ok` in
  // subsequent Node-API function calls, so this would have previously thrown an
  // `Error` object instead of a `TypeError` object.
  Env env = info.Env();
  bool res;
  napi_get_value_bool(env, Value::From(env, "asd"), &res);
  NAPI_THROW_VOID(Error::New(env));
}

void TestErrorCopySemantics(const Napi::CallbackInfo& info) {
  Napi::Error newError = Napi::Error::New(info.Env(), "errorCopyCtor");
  Napi::Error existingErr;

#ifdef NAPI_CPP_EXCEPTIONS
  std::string msg = "errorCopyCtor";
  assert(strcmp(newError.what(), msg.c_str()) == 0);
#endif

  Napi::Error errCopyCtor = newError;
  assert(errCopyCtor.Message() == "errorCopyCtor");

  existingErr = newError;
  assert(existingErr.Message() == "errorCopyCtor");
}

void TestErrorMoveSemantics(const Napi::CallbackInfo& info) {
  std::string errorMsg = "errorMoveCtor";
  Napi::Error newError = Napi::Error::New(info.Env(), errorMsg.c_str());
  Napi::Error errFromMove = std::move(newError);
  assert(errFromMove.Message() == "errorMoveCtor");

  newError = Napi::Error::New(info.Env(), "errorMoveAssign");
  Napi::Error existingErr = std::move(newError);

  assert(existingErr.Message() == "errorMoveAssign");
}

#ifdef NAPI_CPP_EXCEPTIONS

void ThrowJSError(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  throw Error::New(info.Env(), message);
}

void ThrowTypeErrorCtor(const CallbackInfo& info) {
  Napi::Value js_type_error = info[0];
  ReleaseAndWaitForChildProcess(info, 1);

  throw Napi::TypeError(info.Env(), js_type_error);
}

void ThrowTypeError(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  throw TypeError::New(info.Env(), message);
}

void ThrowTypeErrorCStr(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  throw TypeError::New(info.Env(), message.c_str());
}

void ThrowRangeErrorCStr(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();
  ReleaseAndWaitForChildProcess(info, 1);
  throw RangeError::New(info.Env(), message.c_str());
}

void ThrowRangeErrorCtor(const CallbackInfo& info) {
  Napi::Value js_range_err = info[0];
  ReleaseAndWaitForChildProcess(info, 1);
  throw Napi::RangeError(info.Env(), js_range_err);
}

void ThrowEmptyRangeError(const CallbackInfo& info) {
  ReleaseAndWaitForChildProcess(info, 1);
  throw RangeError();
}

void ThrowRangeError(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  throw RangeError::New(info.Env(), message);
}

#if NAPI_VERSION > 8
void ThrowSyntaxErrorCStr(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();
  ReleaseAndWaitForChildProcess(info, 1);
  throw SyntaxError::New(info.Env(), message.c_str());
}

void ThrowSyntaxErrorCtor(const CallbackInfo& info) {
  Napi::Value js_range_err = info[0];
  ReleaseAndWaitForChildProcess(info, 1);
  throw Napi::SyntaxError(info.Env(), js_range_err);
}

void ThrowSyntaxError(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  throw SyntaxError::New(info.Env(), message);
}
#endif  // NAPI_VERSION > 8

Value CatchError(const CallbackInfo& info) {
  Function thrower = info[0].As<Function>();
  try {
    thrower({});
  } catch (const Error& e) {
    return e.Value();
  }
  return info.Env().Null();
}

Value CatchErrorMessage(const CallbackInfo& info) {
  Function thrower = info[0].As<Function>();
  try {
    thrower({});
  } catch (const Error& e) {
    std::string message = e.Message();
    return String::New(info.Env(), message);
  }
  return info.Env().Null();
}

void CatchAndRethrowError(const CallbackInfo& info) {
  Function thrower = info[0].As<Function>();
  try {
    thrower({});
  } catch (Error& e) {
    e.Set("caught", Boolean::New(info.Env(), true));
    throw;
  }
}

void ThrowErrorThatEscapesScope(const CallbackInfo& info) {
  HandleScope scope(info.Env());

  std::string message = info[0].As<String>().Utf8Value();
  throw Error::New(info.Env(), message);
}

void CatchAndRethrowErrorThatEscapesScope(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  try {
    ThrowErrorThatEscapesScope(info);
  } catch (Error& e) {
    e.Set("caught", Boolean::New(info.Env(), true));
    throw;
  }
}

#else  // NAPI_CPP_EXCEPTIONS

void ThrowJSError(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  Error::New(info.Env(), message).ThrowAsJavaScriptException();
}

void ThrowTypeError(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  TypeError::New(info.Env(), message).ThrowAsJavaScriptException();
}

void ThrowTypeErrorCtor(const CallbackInfo& info) {
  Napi::Value js_type_error = info[0];
  ReleaseAndWaitForChildProcess(info, 1);
  TypeError(info.Env(), js_type_error).ThrowAsJavaScriptException();
}

void ThrowTypeErrorCStr(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  TypeError::New(info.Env(), message.c_str()).ThrowAsJavaScriptException();
}

void ThrowRangeError(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  RangeError::New(info.Env(), message).ThrowAsJavaScriptException();
}

void ThrowRangeErrorCtor(const CallbackInfo& info) {
  Napi::Value js_range_err = info[0];
  ReleaseAndWaitForChildProcess(info, 1);
  RangeError(info.Env(), js_range_err).ThrowAsJavaScriptException();
}

void ThrowRangeErrorCStr(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();
  ReleaseAndWaitForChildProcess(info, 1);
  RangeError::New(info.Env(), message.c_str()).ThrowAsJavaScriptException();
}

// TODO: Figure out the correct api for this
void ThrowEmptyRangeError(const CallbackInfo& info) {
  ReleaseAndWaitForChildProcess(info, 1);
  RangeError().ThrowAsJavaScriptException();
}

#if NAPI_VERSION > 8
void ThrowSyntaxError(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();

  ReleaseAndWaitForChildProcess(info, 1);
  SyntaxError::New(info.Env(), message).ThrowAsJavaScriptException();
}

void ThrowSyntaxErrorCtor(const CallbackInfo& info) {
  Napi::Value js_range_err = info[0];
  ReleaseAndWaitForChildProcess(info, 1);
  SyntaxError(info.Env(), js_range_err).ThrowAsJavaScriptException();
}

void ThrowSyntaxErrorCStr(const CallbackInfo& info) {
  std::string message = info[0].As<String>().Utf8Value();
  ReleaseAndWaitForChildProcess(info, 1);
  SyntaxError::New(info.Env(), message.c_str()).ThrowAsJavaScriptException();
}
#endif  // NAPI_VERSION > 8

Value CatchError(const CallbackInfo& info) {
  Function thrower = info[0].As<Function>();
  thrower({});

  Env env = info.Env();
  if (env.IsExceptionPending()) {
    Error e = env.GetAndClearPendingException();
    return e.Value();
  }
  return info.Env().Null();
}

Value CatchErrorMessage(const CallbackInfo& info) {
  Function thrower = info[0].As<Function>();
  thrower({});

  Env env = info.Env();
  if (env.IsExceptionPending()) {
    Error e = env.GetAndClearPendingException();
    std::string message = e.Message();
    return String::New(env, message);
  }
  return info.Env().Null();
}

void CatchAndRethrowError(const CallbackInfo& info) {
  Function thrower = info[0].As<Function>();
  thrower({});

  Env env = info.Env();
  if (env.IsExceptionPending()) {
    Error e = env.GetAndClearPendingException();
    e.Set("caught", Boolean::New(info.Env(), true));
    e.ThrowAsJavaScriptException();
  }
}

void ThrowErrorThatEscapesScope(const CallbackInfo& info) {
  HandleScope scope(info.Env());

  std::string message = info[0].As<String>().Utf8Value();
  Error::New(info.Env(), message).ThrowAsJavaScriptException();
}

void CatchAndRethrowErrorThatEscapesScope(const CallbackInfo& info) {
  HandleScope scope(info.Env());
  ThrowErrorThatEscapesScope(info);

  Env env = info.Env();
  if (env.IsExceptionPending()) {
    Error e = env.GetAndClearPendingException();
    e.Set("caught", Boolean::New(info.Env(), true));
    e.ThrowAsJavaScriptException();
  }
}

#endif  // NAPI_CPP_EXCEPTIONS

void ThrowFatalError(const CallbackInfo& /*info*/) {
  Error::Fatal("Error::ThrowFatalError", "This is a fatal error");
}

void ThrowDefaultError(const CallbackInfo& info) {
  napi_value dummy;
  napi_env env = info.Env();
  napi_status status = napi_get_undefined(env, &dummy);
  NAPI_FATAL_IF_FAILED(status, "ThrowDefaultError", "napi_get_undefined");

  if (info[0].As<Boolean>().Value()) {
    // Provoke Node-API into setting an error, then use the `Napi::Error::New`
    // factory with only the `env` parameter to throw an exception generated
    // from the last error.
    uint32_t dummy_uint32;
    status = napi_get_value_uint32(env, dummy, &dummy_uint32);
    if (status == napi_ok) {
      Error::Fatal("ThrowDefaultError", "napi_get_value_uint32");
    }
    // We cannot use `NAPI_THROW_IF_FAILED()` here because we do not wish
    // control to pass back to the engine if we throw an exception here and C++
    // exceptions are turned on.
    Napi::Error::New(env).ThrowAsJavaScriptException();
  }

  // Produce and throw a second error that has different content than the one
  // above. If the one above was thrown, then throwing the one below should
  // have the effect of re-throwing the one above.
  status = napi_get_named_property(env, dummy, "xyzzy", &dummy);
  if (status == napi_ok) {
    Error::Fatal("ThrowDefaultError", "napi_get_named_property");
  }

  ReleaseAndWaitForChildProcess(info, 1);

  // The macro creates a `Napi::Error` using the factory that takes only the
  // env, however, it heeds the exception mechanism to be used.
  NAPI_THROW_IF_FAILED_VOID(env, status);
}

}  // end anonymous namespace

Object InitError(Env env) {
  Object exports = Object::New(env);
  exports["throwApiError"] = Function::New(env, ThrowApiError);
  exports["testErrorCopySemantics"] =
      Function::New(env, TestErrorCopySemantics);
  exports["testErrorMoveSemantics"] =
      Function::New(env, TestErrorMoveSemantics);
  exports["lastExceptionErrorCode"] =
      Function::New(env, LastExceptionErrorCode);
  exports["throwJSError"] = Function::New(env, ThrowJSError);
  exports["throwTypeError"] = Function::New(env, ThrowTypeError);
  exports["throwTypeErrorCtor"] = Function::New(env, ThrowTypeErrorCtor);
  exports["throwTypeErrorCStr"] = Function::New(env, ThrowTypeErrorCStr);
  exports["throwRangeError"] = Function::New(env, ThrowRangeError);
  exports["throwRangeErrorCtor"] = Function::New(env, ThrowRangeErrorCtor);
  exports["throwRangeErrorCStr"] = Function::New(env, ThrowRangeErrorCStr);
  exports["throwEmptyRangeError"] = Function::New(env, ThrowEmptyRangeError);
#if NAPI_VERSION > 8
  exports["throwSyntaxError"] = Function::New(env, ThrowSyntaxError);
  exports["throwSyntaxErrorCtor"] = Function::New(env, ThrowSyntaxErrorCtor);
  exports["throwSyntaxErrorCStr"] = Function::New(env, ThrowSyntaxErrorCStr);
#endif  // NAPI_VERSION > 8
  exports["catchError"] = Function::New(env, CatchError);
  exports["catchErrorMessage"] = Function::New(env, CatchErrorMessage);
  exports["doNotCatch"] = Function::New(env, DoNotCatch);
  exports["catchAndRethrowError"] = Function::New(env, CatchAndRethrowError);
  exports["throwErrorThatEscapesScope"] =
      Function::New(env, ThrowErrorThatEscapesScope);
  exports["catchAndRethrowErrorThatEscapesScope"] =
      Function::New(env, CatchAndRethrowErrorThatEscapesScope);
  exports["throwFatalError"] = Function::New(env, ThrowFatalError);
  exports["throwDefaultError"] = Function::New(env, ThrowDefaultError);
  exports["resetPromises"] = Function::New(env, ResetPromises);
  exports["waitForWorkerThread"] = Function::New(env, WaitForWorkerThread);
  exports["releaseWorkerThread"] = Function::New(env, ReleaseWorkerThread);
  return exports;
}
