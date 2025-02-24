#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "napi.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

constexpr size_t ARRAY_LENGTH = 10;
constexpr size_t MAX_QUEUE_SIZE = 2;

static std::thread threads[2];

static struct ThreadSafeFunctionInfo {
  enum CallType { DEFAULT, BLOCKING, NON_BLOCKING } type;
  bool abort;
  bool startSecondary;
  FunctionReference jsFinalizeCallback;
  uint32_t maxQueueSize;
  bool closeCalledFromJs;
  std::mutex protect;
  std::condition_variable signal;
} tsfnInfo;

static void TSFNCallJS(Env env,
                       Function jsCallback,
                       ThreadSafeFunctionInfo* /* context */,
                       int* data) {
  // A null environment signifies the threadsafe function has been finalized.
  if (!(env == nullptr || jsCallback == nullptr)) {
    // If called with no data
    if (data == nullptr) {
      jsCallback.Call({});
    } else {
      jsCallback.Call({Number::New(env, *data)});
    }
  }
}

using TSFN = TypedThreadSafeFunction<ThreadSafeFunctionInfo, int, TSFNCallJS>;
static TSFN s_tsfn;

// Thread data to transmit to JS
static int ints[ARRAY_LENGTH];

static void SecondaryThread() {
  if (s_tsfn.Release() != napi_ok) {
    Error::Fatal("TypedSecondaryThread", "ThreadSafeFunction.Release() failed");
  }
}

// Source thread producing the data
static void DataSourceThread() {
  ThreadSafeFunctionInfo* info = s_tsfn.GetContext();

  if (info->startSecondary) {
    if (s_tsfn.Acquire() != napi_ok) {
      Error::Fatal("TypedDataSourceThread",
                   "ThreadSafeFunction.Acquire() failed");
    }

    threads[1] = std::thread(SecondaryThread);
  }

  bool queueWasFull = false;
  bool queueWasClosing = false;
  for (int index = ARRAY_LENGTH - 1; index > -1 && !queueWasClosing; index--) {
    napi_status status = napi_generic_failure;

    switch (info->type) {
      case ThreadSafeFunctionInfo::DEFAULT:
        status = s_tsfn.BlockingCall();
        break;
      case ThreadSafeFunctionInfo::BLOCKING:
        status = s_tsfn.BlockingCall(&ints[index]);
        break;
      case ThreadSafeFunctionInfo::NON_BLOCKING:
        status = s_tsfn.NonBlockingCall(&ints[index]);
        break;
    }

    if (info->abort && info->type != ThreadSafeFunctionInfo::NON_BLOCKING) {
      // Let's make this thread really busy to give the main thread a chance to
      // abort / close.
      std::unique_lock<std::mutex> lk(info->protect);
      while (!info->closeCalledFromJs) {
        info->signal.wait(lk);
      }
    }

    switch (status) {
      case napi_queue_full:
        queueWasFull = true;
        index++;
        // fall through

      case napi_ok:
        continue;

      case napi_closing:
        queueWasClosing = true;
        break;

      default:
        Error::Fatal("TypedDataSourceThread",
                     "ThreadSafeFunction.*Call() failed");
    }
  }

  if (info->type == ThreadSafeFunctionInfo::NON_BLOCKING && !queueWasFull) {
    Error::Fatal("TypedDataSourceThread", "Queue was never full");
  }

  if (info->abort && !queueWasClosing) {
    Error::Fatal("TypedDataSourceThread", "Queue was never closing");
  }

  if (!queueWasClosing && s_tsfn.Release() != napi_ok) {
    Error::Fatal("TypedDataSourceThread",
                 "ThreadSafeFunction.Release() failed");
  }
}

static Value StopThread(const CallbackInfo& info) {
  tsfnInfo.jsFinalizeCallback = Napi::Persistent(info[0].As<Function>());
  bool abort = info[1].As<Boolean>();
  if (abort) {
    s_tsfn.Abort();
  } else {
    s_tsfn.Release();
  }
  {
    std::lock_guard<std::mutex> _(tsfnInfo.protect);
    tsfnInfo.closeCalledFromJs = true;
    tsfnInfo.signal.notify_one();
  }
  return Value();
}

// Join the thread and inform JS that we're done.
static void JoinTheThreads(Env /* env */,
                           std::thread* theThreads,
                           ThreadSafeFunctionInfo* info) {
  theThreads[0].join();
  if (info->startSecondary) {
    theThreads[1].join();
  }

  info->jsFinalizeCallback.Call({});
  info->jsFinalizeCallback.Reset();
}

static Value StartThreadInternal(const CallbackInfo& info,
                                 ThreadSafeFunctionInfo::CallType type) {
  tsfnInfo.type = type;
  tsfnInfo.abort = info[1].As<Boolean>();
  tsfnInfo.startSecondary = info[2].As<Boolean>();
  tsfnInfo.maxQueueSize = info[3].As<Number>().Uint32Value();
  tsfnInfo.closeCalledFromJs = false;

  s_tsfn = TSFN::New(info.Env(),
                     info[0].As<Function>(),
                     Object::New(info.Env()),
                     "Test",
                     tsfnInfo.maxQueueSize,
                     2,
                     &tsfnInfo,
                     JoinTheThreads,
                     threads);

  threads[0] = std::thread(DataSourceThread);

  return Value();
}

static Value Release(const CallbackInfo& /* info */) {
  if (s_tsfn.Release() != napi_ok) {
    Error::Fatal("Release", "TypedThreadSafeFunction.Release() failed");
  }
  return Value();
}

static Value StartThread(const CallbackInfo& info) {
  return StartThreadInternal(info, ThreadSafeFunctionInfo::BLOCKING);
}

static Value StartThreadNonblocking(const CallbackInfo& info) {
  return StartThreadInternal(info, ThreadSafeFunctionInfo::NON_BLOCKING);
}

static Value StartThreadNoNative(const CallbackInfo& info) {
  return StartThreadInternal(info, ThreadSafeFunctionInfo::DEFAULT);
}

Object InitTypedThreadSafeFunction(Env env) {
  for (size_t index = 0; index < ARRAY_LENGTH; index++) {
    ints[index] = index;
  }

  Object exports = Object::New(env);
  exports["ARRAY_LENGTH"] = Number::New(env, ARRAY_LENGTH);
  exports["MAX_QUEUE_SIZE"] = Number::New(env, MAX_QUEUE_SIZE);
  exports["startThread"] = Function::New(env, StartThread);
  exports["startThreadNoNative"] = Function::New(env, StartThreadNoNative);
  exports["startThreadNonblocking"] =
      Function::New(env, StartThreadNonblocking);
  exports["stopThread"] = Function::New(env, StopThread);
  exports["release"] = Function::New(env, Release);

  return exports;
}

#endif
