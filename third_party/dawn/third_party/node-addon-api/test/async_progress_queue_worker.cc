#include "napi.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

struct ProgressData {
  int32_t progress;
};

class TestWorkerWithNoCb : public AsyncProgressQueueWorker<ProgressData> {
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
  TestWorkerWithNoCb(Napi::Env env, Function cb)
      : AsyncProgressQueueWorker(env) {
    _cb.Reset(cb, 1);
  }
  TestWorkerWithNoCb(Napi::Env env, const char* resourceName, Function cb)
      : AsyncProgressQueueWorker(env, resourceName) {
    _cb.Reset(cb, 1);
  }
  TestWorkerWithNoCb(Napi::Env env,
                     const char* resourceName,
                     const Object& resourceObject,
                     Function cb)
      : AsyncProgressQueueWorker(env, resourceName, resourceObject) {
    _cb.Reset(cb, 1);
  }
  FunctionReference _cb;
};

class TestWorkerWithRecv : public AsyncProgressQueueWorker<ProgressData> {
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
      : AsyncProgressQueueWorker(recv, cb) {}
  TestWorkerWithRecv(const Object& recv,
                     const Function& cb,
                     const char* resourceName)
      : AsyncProgressQueueWorker(recv, cb, resourceName) {}
  TestWorkerWithRecv(const Object& recv,
                     const Function& cb,
                     const char* resourceName,
                     const Object& resourceObject)
      : AsyncProgressQueueWorker(recv, cb, resourceName, resourceObject) {}
};

class TestWorkerWithCb : public AsyncProgressQueueWorker<ProgressData> {
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
  TestWorkerWithCb(Function cb) : AsyncProgressQueueWorker(cb) {}
  TestWorkerWithCb(Function cb, const char* res_name)
      : AsyncProgressQueueWorker(cb, res_name) {}
};

class TestWorker : public AsyncProgressQueueWorker<ProgressData> {
 public:
  static Napi::Value CreateWork(const CallbackInfo& info) {
    int32_t times = info[0].As<Number>().Int32Value();
    Function cb = info[1].As<Function>();
    Function progress = info[2].As<Function>();

    TestWorker* worker = new TestWorker(
        cb, progress, "TestResource", Object::New(info.Env()), times);

    return Napi::External<TestWorker>::New(info.Env(), worker);
  }

  static void QueueWork(const CallbackInfo& info) {
    auto wrap = info[0].As<Napi::External<TestWorker>>();
    auto worker = wrap.Data();
    worker->Queue();
  }

 protected:
  void Execute(const ExecutionProgress& progress) override {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    if (_times < 0) {
      SetError("test error");
    } else {
      progress.Signal();
    }
    ProgressData data{0};
    for (int32_t idx = 0; idx < _times; idx++) {
      data.progress = idx;
      progress.Send(&data, 1);
    }
  }

  void OnProgress(const ProgressData* data, size_t count) override {
    Napi::Env env = Env();
    _test_case_count++;
    if (!_js_progress_cb.IsEmpty()) {
      if (_test_case_count == 1) {
        if (count != 0) {
          SetError("expect 0 count of data on 1st call");
        }
      } else {
        Number progress = Number::New(env, data->progress);
        _js_progress_cb.Call(Receiver().Value(), {progress});
      }
    }
  }

 private:
  TestWorker(Function cb,
             Function progress,
             const char* resource_name,
             const Object& resource,
             int32_t times)
      : AsyncProgressQueueWorker(cb, resource_name, resource), _times(times) {
    _js_progress_cb.Reset(progress, 1);
  }

  int32_t _times;
  size_t _test_case_count = 0;
  FunctionReference _js_progress_cb;
};

}  // namespace

Object InitAsyncProgressQueueWorker(Env env) {
  Object exports = Object::New(env);
  exports["createWork"] = Function::New(env, TestWorker::CreateWork);
  exports["queueWork"] = Function::New(env, TestWorker::QueueWork);
  exports["runWorkerNoCb"] = Function::New(env, TestWorkerWithNoCb::DoWork);
  exports["runWorkerWithRecv"] = Function::New(env, TestWorkerWithRecv::DoWork);
  exports["runWorkerWithCb"] = Function::New(env, TestWorkerWithCb::DoWork);
  return exports;
}

#endif
