#include "napi.h"

using namespace Napi;

#if (NAPI_VERSION > 5)
Object InitAddon(Env env);
Object InitAddonData(Env env);
#endif
Object InitArrayBuffer(Env env);
Object InitAsyncContext(Env env);
#if (NAPI_VERSION > 3)
Object InitAsyncProgressQueueWorker(Env env);
Object InitAsyncProgressWorker(Env env);
#endif
Object InitAsyncWorker(Env env);
Object InitPersistentAsyncWorker(Env env);
Object InitBasicTypesArray(Env env);
Object InitBasicTypesBoolean(Env env);
Object InitBasicTypesNumber(Env env);
Object InitBasicTypesValue(Env env);
#if (NAPI_VERSION > 5)
Object InitBigInt(Env env);
#endif
Object InitBuffer(Env env);
Object InitBufferNoExternal(Env env);
#if (NAPI_VERSION > 2)
Object InitCallbackScope(Env env);
#endif
#if (NAPI_VERSION > 4)
Object InitDate(Env env);
#endif
Object InitCallbackInfo(Env env);
Object InitDataView(Env env);
Object InitDataViewReadWrite(Env env);
Object InitEnvCleanup(Env env);
Object InitErrorHandlingPrim(Env env);
Object InitError(Env env);
Object InitExternal(Env env);
Object InitFunction(Env env);
Object InitFunctionReference(Env env);
Object InitHandleScope(Env env);
Object InitMovableCallbacks(Env env);
Object InitMemoryManagement(Env env);
Object InitName(Env env);
Object InitObject(Env env);
#ifndef NODE_ADDON_API_DISABLE_DEPRECATED
Object InitObjectDeprecated(Env env);
#endif  // !NODE_ADDON_API_DISABLE_DEPRECATED
Object InitPromise(Env env);
Object InitRunScript(Env env);
#if (NAPI_VERSION > 3)
Object InitThreadSafeFunctionCtx(Env env);
Object InitThreadSafeFunctionException(Env env);
Object InitThreadSafeFunctionExistingTsfn(Env env);
Object InitThreadSafeFunctionPtr(Env env);
Object InitThreadSafeFunctionSum(Env env);
Object InitThreadSafeFunctionUnref(Env env);
Object InitThreadSafeFunction(Env env);
Object InitTypedThreadSafeFunctionCtx(Env env);
Object InitTypedThreadSafeFunctionException(Env env);
Object InitTypedThreadSafeFunctionExistingTsfn(Env env);
Object InitTypedThreadSafeFunctionPtr(Env env);
Object InitTypedThreadSafeFunctionSum(Env env);
Object InitTypedThreadSafeFunctionUnref(Env env);
Object InitTypedThreadSafeFunction(Env env);
#endif
Object InitSymbol(Env env);
Object InitTypedArray(Env env);
Object InitGlobalObject(Env env);
Object InitObjectWrap(Env env);
Object InitObjectWrapConstructorException(Env env);
Object InitObjectWrapFunction(Env env);
Object InitObjectWrapRemoveWrap(Env env);
Object InitObjectWrapMultipleInheritance(Env env);
Object InitObjectReference(Env env);
Object InitReference(Env env);
Object InitVersionManagement(Env env);
Object InitThunkingManual(Env env);
#if (NAPI_VERSION > 7)
Object InitObjectFreezeSeal(Env env);
Object InitTypeTaggable(Env env);
#endif
#if (NAPI_VERSION > 8)
Object InitEnvMiscellaneous(Env env);
#endif
#if defined(NODE_ADDON_API_ENABLE_MAYBE)
Object InitMaybeCheck(Env env);
#endif

Object Init(Env env, Object exports) {
#if (NAPI_VERSION > 5)
  exports.Set("addon", InitAddon(env));
  exports.Set("addon_data", InitAddonData(env));
#endif
  exports.Set("arraybuffer", InitArrayBuffer(env));
  exports.Set("asynccontext", InitAsyncContext(env));
#if (NAPI_VERSION > 3)
  exports.Set("asyncprogressqueueworker", InitAsyncProgressQueueWorker(env));
  exports.Set("asyncprogressworker", InitAsyncProgressWorker(env));
#endif
  exports.Set("globalObject", InitGlobalObject(env));
  exports.Set("asyncworker", InitAsyncWorker(env));
  exports.Set("persistentasyncworker", InitPersistentAsyncWorker(env));
  exports.Set("basic_types_array", InitBasicTypesArray(env));
  exports.Set("basic_types_boolean", InitBasicTypesBoolean(env));
  exports.Set("basic_types_number", InitBasicTypesNumber(env));
  exports.Set("basic_types_value", InitBasicTypesValue(env));
#if (NAPI_VERSION > 5)
  exports.Set("bigint", InitBigInt(env));
#endif
#if (NAPI_VERSION > 4)
  exports.Set("date", InitDate(env));
#endif
  exports.Set("buffer", InitBuffer(env));
  exports.Set("bufferNoExternal", InitBufferNoExternal(env));
#if (NAPI_VERSION > 2)
  exports.Set("callbackscope", InitCallbackScope(env));
#endif
  exports.Set("callbackInfo", InitCallbackInfo(env));
  exports.Set("dataview", InitDataView(env));
  exports.Set("dataview_read_write", InitDataView(env));
  exports.Set("dataview_read_write", InitDataViewReadWrite(env));
#if (NAPI_VERSION > 2)
  exports.Set("env_cleanup", InitEnvCleanup(env));
#endif
  exports.Set("error", InitError(env));
  exports.Set("errorHandlingPrim", InitErrorHandlingPrim(env));
  exports.Set("external", InitExternal(env));
  exports.Set("function", InitFunction(env));
  exports.Set("functionreference", InitFunctionReference(env));
  exports.Set("name", InitName(env));
  exports.Set("handlescope", InitHandleScope(env));
  exports.Set("movable_callbacks", InitMovableCallbacks(env));
  exports.Set("memory_management", InitMemoryManagement(env));
  exports.Set("object", InitObject(env));
#ifndef NODE_ADDON_API_DISABLE_DEPRECATED
  exports.Set("object_deprecated", InitObjectDeprecated(env));
#endif  // !NODE_ADDON_API_DISABLE_DEPRECATED
  exports.Set("promise", InitPromise(env));
  exports.Set("run_script", InitRunScript(env));
  exports.Set("symbol", InitSymbol(env));
#if (NAPI_VERSION > 3)
  exports.Set("threadsafe_function_ctx", InitThreadSafeFunctionCtx(env));
  exports.Set("threadsafe_function_exception",
              InitThreadSafeFunctionException(env));
  exports.Set("threadsafe_function_existing_tsfn",
              InitThreadSafeFunctionExistingTsfn(env));
  exports.Set("threadsafe_function_ptr", InitThreadSafeFunctionPtr(env));
  exports.Set("threadsafe_function_sum", InitThreadSafeFunctionSum(env));
  exports.Set("threadsafe_function_unref", InitThreadSafeFunctionUnref(env));
  exports.Set("threadsafe_function", InitThreadSafeFunction(env));
  exports.Set("typed_threadsafe_function_ctx",
              InitTypedThreadSafeFunctionCtx(env));
  exports.Set("typed_threadsafe_function_exception",
              InitTypedThreadSafeFunctionException(env));
  exports.Set("typed_threadsafe_function_existing_tsfn",
              InitTypedThreadSafeFunctionExistingTsfn(env));
  exports.Set("typed_threadsafe_function_ptr",
              InitTypedThreadSafeFunctionPtr(env));
  exports.Set("typed_threadsafe_function_sum",
              InitTypedThreadSafeFunctionSum(env));
  exports.Set("typed_threadsafe_function_unref",
              InitTypedThreadSafeFunctionUnref(env));
  exports.Set("typed_threadsafe_function", InitTypedThreadSafeFunction(env));
#endif
  exports.Set("typedarray", InitTypedArray(env));
  exports.Set("objectwrap", InitObjectWrap(env));
  exports.Set("objectwrapConstructorException",
              InitObjectWrapConstructorException(env));
  exports.Set("objectwrap_function", InitObjectWrapFunction(env));
  exports.Set("objectwrap_removewrap", InitObjectWrapRemoveWrap(env));
  exports.Set("objectwrap_multiple_inheritance",
              InitObjectWrapMultipleInheritance(env));
  exports.Set("objectreference", InitObjectReference(env));
  exports.Set("reference", InitReference(env));
  exports.Set("version_management", InitVersionManagement(env));
  exports.Set("thunking_manual", InitThunkingManual(env));
#if (NAPI_VERSION > 7)
  exports.Set("object_freeze_seal", InitObjectFreezeSeal(env));
  exports.Set("type_taggable", InitTypeTaggable(env));
#endif
#if (NAPI_VERSION > 8)
  exports.Set("env_misc", InitEnvMiscellaneous(env));
#endif

#if defined(NODE_ADDON_API_ENABLE_MAYBE)
  exports.Set("maybe_check", InitMaybeCheck(env));
#endif
  return exports;
}

NODE_API_MODULE(addon, Init)
