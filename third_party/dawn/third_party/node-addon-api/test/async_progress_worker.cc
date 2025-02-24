#include "napi.h"

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

struct ProgressData {
  size_t progress;
};

class TestWorkerWithNoCb : public AsyncProgressWorker<ProgressData> {
 public:
  static void DoWork(const CallbackInfo& info) {
    switch (info.Length()) {
      case 1: {
        Function cb = info[0].As<Function>();
        TestWorkerWithNoCb* worker = new TestWorkerWithNoCb(info.Env(), cb);
        worker->Queue();
      } break;

      case 2: {
        std::string resName = info[0].As<String>();
        Function cb = info[1].As<Function>();
        TestWorkerWithNoCb* worker =
            new TestWorkerWithNoCb(info.Env(), resName.c_str(), cb);
        worker->Queue();
      } break;

      case 3: {
        std::string resName = info[0].As<String>();
        Object resObject = info[1].As<Object>();
        Function cb = info[2].As<Function>();
        TestWorkerWithNoCb* worker =
            new TestWorkerWithNoCb(info.Env(), resName.c_str(), resObject, cb);
        worker->Queue();
      } break;

      default:

        break;
    }
  }

 protected:
  void Execute(const ExecutionProgress& progress) override {
    ProgressData data{1};
    progress.Send(&data, 1);
  }

  void OnProgress(const ProgressData*, size_t /* count */) override {
    _cb.Call({});
  }

 private:
  TestWorkerWithNoCb(Napi::Env env, Function cb) : AsyncProgressWorker(env) {
    _cb.Reset(cb, 1);
  }
  TestWorkerWithNoCb(Napi::Env env, const char* resourceName, Function cb)
      : AsyncProgressWorker(env, resourceName) {
    _cb.Reset(cb, 1);
  }
  TestWorkerWithNoCb(Napi::Env env,
                     const char* resourceName,
                     const Object& resourceObject,
                     Function cb)
      : AsyncProgressWorker(env, resourceName, resourceObject) {
    _cb.Reset(cb, 1);
  }
  FunctionReference _cb;
};

class TestWorkerWithRecv : public AsyncProgressWorker<ProgressData> {
 public:
  static void DoWork(const CallbackInfo& info) {
    switch (info.Length()) {
      case 2: {
        Object recv = info[0].As<Object>();
        Function cb = info[1].As<Function>();
        TestWorkerWithRecv* worker = new TestWorkerWithRecv(recv, cb);
        worker->Queue();
      } break;

      case 3: {
        Object recv = info[0].As<Object>();
        Function cb = info[1].As<Function>();
        std::string resName = info[2].As<String>();
        TestWorkerWithRecv* worker =
            new TestWorkerWithRecv(recv, cb, resName.c_str());
        worker->Queue();
      } break;

      case 4: {
        Object recv = info[0].As<Object>();
        Function cb = info[1].As<Function>();
        std::string resName = info[2].As<String>();
        Object resObject = info[3].As<Object>();
        TestWorkerWithRecv* worker =
            new TestWorkerWithRecv(recv, cb, resName.c_str(), resObject);
        worker->Queue();
      } break;

      default:

        break;
    }
  }

 protected:
  void Execute(const ExecutionProgress&) override {}

  void OnProgress(const ProgressData*, size_t /* count */) override {}

 private:
  TestWorkerWithRecv(const Object& recv, const Function& cb)
      : AsyncProgressWorker(recv, cb) {}
  TestWorkerWithRecv(const Object& recv,
                     const Function& cb,
                     const char* resourceName)
      : AsyncProgressWorker(recv, cb, resourceName) {}
  TestWorkerWithRecv(const Object& recv,
                     const Function& cb,
                     const char* resourceName,
                     const Object& resourceObject)
      : AsyncProgressWorker(recv, cb, resourceName, resourceObject) {}
};

class TestWorkerWithCb : public AsyncProgressWorker<ProgressData> {
 public:
  static void DoWork(const CallbackInfo& info) {
    switch (info.Length()) {
      case 1: {
        Function cb = info[0].As<Function>();
        TestWorkerWithCb* worker = new TestWorkerWithCb(cb);
        worker->Queue();
      } break;

      case 2: {
        Function cb = info[0].As<Function>();
        std::string asyncResName = info[1].As<String>();
        TestWorkerWithCb* worker =
            new TestWorkerWithCb(cb, asyncResName.c_str());
        worker->Queue();
      } break;

      default:

        break;
    }
  }

 protected:
  void Execute(const ExecutionProgress&) override {}

  void OnProgress(const ProgressData*, size_t /* count */) override {}

 private:
  TestWorkerWithCb(Function cb) : AsyncProgressWorker(cb) {}
  TestWorkerWithCb(Function cb, const char* res_name)
      : AsyncProgressWorker(cb, res_name) {}
};

class TestWorker : public AsyncProgressWorker<ProgressData> {
 public:
  static void DoWork(const CallbackInfo& info) {
    int32_t times = info[0].As<Number>().Int32Value();
    Function cb = info[1].As<Function>();
    Function progress = info[2].As<Function>();

    TestWorker* worker =
        new TestWorker(cb, progress, "TestResource", Object::New(info.Env()));
    worker->_times = times;
    worker->Queue();
  }

 protected:
  void Execute(const ExecutionProgress& progress) override {
    if (_times < 0) {
      SetError("test error");
    }
    ProgressData data{0};

    for (int32_t idx = 0; idx < _times; idx++) {
      data.progress = idx;
      progress.Send(&data, 1);

      {
        std::unique_lock<std::mutex> lk(_cvm);
        _cv.wait(lk, [this] { return dataSent; });
        dataSent = false;
      }
    }
  }

  void OnProgress(const ProgressData* data, size_t /* count */) override {
    Napi::Env env = Env();
    if (!_progress.IsEmpty()) {
      Number progress = Number::New(env, data->progress);
      _progress.MakeCallback(Receiver().Value(), {progress});
    }

    {
      std::lock_guard<std::mutex> lk(_cvm);
      dataSent = true;
      _cv.notify_one();
    }
  }

 private:
  TestWorker(Function cb,
             Function progress,
             const char* resource_name,
             const Object& resource)
      : AsyncProgressWorker(cb, resource_name, resource) {
    _progress.Reset(progress, 1);
  }

  bool dataSent = false;
  std::condition_variable _cv;
  std::mutex _cvm;
  int32_t _times;
  FunctionReference _progress;
};

class MalignWorker : public AsyncProgressWorker<ProgressData> {
 public:
  static void DoWork(const CallbackInfo& info) {
    Function cb = info[0].As<Function>();
    Function progress = info[1].As<Function>();

    MalignWorker* worker =
        new MalignWorker(cb, progress, "TestResource", Object::New(info.Env()));
    worker->Queue();
  }

 protected:
  void Execute(const ExecutionProgress& progress) override {
    {
      std::unique_lock<std::mutex> lock(_cvm);
      // Testing a nullptr send is acceptable.
      progress.Send(nullptr, 0);
      _cv.wait(lock, [this] { return _test_case_count == 1; });
    }
    {
      std::unique_lock<std::mutex> lock(_cvm);
      progress.Signal();
      _cv.wait(lock, [this] { return _test_case_count == 2; });
    }
    // Testing busy looping on send doesn't trigger unexpected empty data
    // OnProgress call.
    for (size_t i = 0; i < 1000000; i++) {
      ProgressData data{0};
      progress.Send(&data, 1);
    }
  }

  void OnProgress(const ProgressData* /* data */, size_t count) override {
    Napi::Env env = Env();
    {
      std::lock_guard<std::mutex> lock(_cvm);
      _test_case_count++;
    }
    bool error = false;
    Napi::String reason = Napi::String::New(env, "No error");
    if (_test_case_count <= 2 && count != 0) {
      error = true;
      reason =
          Napi::String::New(env, "expect 0 count of data on 1st and 2nd call");
    }
    if (_test_case_count > 2 && count != 1) {
      error = true;
      reason = Napi::String::New(
          env, "expect 1 count of data on non-1st and non-2nd call");
    }
    _progress.MakeCallback(Receiver().Value(),
                           {Napi::Boolean::New(env, error), reason});
    _cv.notify_one();
  }

 private:
  MalignWorker(Function cb,
               Function progress,
               const char* resource_name,
               const Object& resource)
      : AsyncProgressWorker(cb, resource_name, resource) {
    _progress.Reset(progress, 1);
  }

  size_t _test_case_count = 0;
  std::condition_variable _cv;
  std::mutex _cvm;
  FunctionReference _progress;
};

// Calling a Signal after a SendProgress should not clear progress data
class SignalAfterProgressTestWorker : public AsyncProgressWorker<ProgressData> {
 public:
  static void DoWork(const CallbackInfo& info) {
    Function cb = info[0].As<Function>();
    Function progress = info[1].As<Function>();

    SignalAfterProgressTestWorker* worker = new SignalAfterProgressTestWorker(
        cb, progress, "TestResource", Object::New(info.Env()));
    worker->Queue();
  }

 protected:
  void Execute(const ExecutionProgress& progress) override {
    ProgressData data{0};
    progress.Send(&data, 1);
    progress.Signal();
  }

  void OnProgress(const ProgressData* /* data */, size_t count) override {
    Napi::Env env = Env();
    bool error = false;
    Napi::String reason = Napi::String::New(env, "No error");
    if (count != 1) {
      error = true;
      reason = Napi::String::New(env, "expect 1 count of data");
    }
    _progress.MakeCallback(Receiver().Value(),
                           {Napi::Boolean::New(env, error), reason});
  }

 private:
  SignalAfterProgressTestWorker(Function cb,
                                Function progress,
                                const char* resource_name,
                                const Object& resource)
      : AsyncProgressWorker(cb, resource_name, resource) {
    _progress.Reset(progress, 1);
  }
  FunctionReference _progress;
};
}  // namespace

Object InitAsyncProgressWorker(Env env) {
  Object exports = Object::New(env);
  exports["doWork"] = Function::New(env, TestWorker::DoWork);
  exports["doMalignTest"] = Function::New(env, MalignWorker::DoWork);
  exports["doSignalAfterProgressTest"] =
      Function::New(env, SignalAfterProgressTestWorker::DoWork);
  exports["runWorkerNoCb"] = Function::New(env, TestWorkerWithNoCb::DoWork);
  exports["runWorkerWithRecv"] = Function::New(env, TestWorkerWithRecv::DoWork);
  exports["runWorkerWithCb"] = Function::New(env, TestWorkerWithCb::DoWork);
  return exports;
}

#endif
