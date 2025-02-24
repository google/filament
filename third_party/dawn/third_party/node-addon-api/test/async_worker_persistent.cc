#include "napi.h"

// A variant of TestWorker wherein destruction is suppressed. That is, instances
// are not destroyed during the `OnOK` callback. They must be explicitly
// destroyed.

using namespace Napi;

namespace {

class PersistentTestWorker : public AsyncWorker {
 public:
  static PersistentTestWorker* current_worker;
  static void DoWork(const CallbackInfo& info) {
    bool succeed = info[0].As<Boolean>();
    Function cb = info[1].As<Function>();

    PersistentTestWorker* worker = new PersistentTestWorker(cb, "TestResource");
    current_worker = worker;

    worker->SuppressDestruct();
    worker->_succeed = succeed;
    worker->Queue();
  }

  static Value GetWorkerGone(const CallbackInfo& info) {
    return Boolean::New(info.Env(), current_worker == nullptr);
  }

  static void DeleteWorker(const CallbackInfo& info) {
    (void)info;
    delete current_worker;
  }

  ~PersistentTestWorker() { current_worker = nullptr; }

 protected:
  void Execute() override {
    if (!_succeed) {
      SetError("test error");
    }
  }

 private:
  PersistentTestWorker(Function cb, const char* resource_name)
      : AsyncWorker(cb, resource_name) {}

  bool _succeed;
};

PersistentTestWorker* PersistentTestWorker::current_worker = nullptr;

}  // end of anonymous namespace

Object InitPersistentAsyncWorker(Env env) {
  Object exports = Object::New(env);
  exports["doWork"] = Function::New(env, PersistentTestWorker::DoWork);
  exports.DefineProperty(PropertyDescriptor::Accessor(
      env, exports, "workerGone", PersistentTestWorker::GetWorkerGone));
  exports["deleteWorker"] =
      Function::New(env, PersistentTestWorker::DeleteWorker);
  return exports;
}
