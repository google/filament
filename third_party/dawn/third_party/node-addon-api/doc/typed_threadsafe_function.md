# TypedThreadSafeFunction

The `Napi::TypedThreadSafeFunction` type provides APIs for threads to
communicate with the addon's main thread to invoke JavaScript functions on their
behalf. The type is a three-argument templated class, each argument representing
the type of:
- `ContextType = std::nullptr_t`: The thread-safe function's context. By
  default, a TSFN has no context.
- `DataType = void*`: The data to use in the native callback. By default, a TSFN
  can accept any data type.
- `Callback = void(*)(Napi::Env, Napi::Function jsCallback, ContextType*,
  DataType*)`: The callback to run for each item added to the queue. If no
  `Callback` is given, the API will call the function `jsCallback` with no
  arguments.

Documentation can be found for an [overview of the API](threadsafe.md), as well
as [differences between the two thread-safe function
APIs](threadsafe.md#implementation-differences).

## Methods

### Constructor

Creates a new empty instance of `Napi::TypedThreadSafeFunction`.

```cpp
Napi::Function::TypedThreadSafeFunction<ContextType, DataType, Callback>::TypedThreadSafeFunction();
```

### Constructor

Creates a new instance of the `Napi::TypedThreadSafeFunction` object.

```cpp
Napi::TypedThreadSafeFunction<ContextType, DataType, Callback>::TypedThreadSafeFunction(napi_threadsafe_function tsfn);
```

- `tsfn`: The `napi_threadsafe_function` which is a handle for an existing
  thread-safe function.

Returns a non-empty `Napi::TypedThreadSafeFunction` instance. To ensure the API
statically handles the correct return type for `GetContext()` and
`[Non]BlockingCall()`, pass the proper template arguments to
`Napi::TypedThreadSafeFunction`.

### New

Creates a new instance of the `Napi::TypedThreadSafeFunction` object. The `New`
function has several overloads for the various optional parameters: skip the
optional parameter for that specific overload.

```cpp
New(napi_env env,
    CallbackType callback,
    const Object& resource,
    ResourceString resourceName,
    size_t maxQueueSize,
    size_t initialThreadCount,
    ContextType* context,
    Finalizer finalizeCallback,
    FinalizerDataType* data = nullptr);
```

- `env`: The `napi_env` environment in which to construct the
  `Napi::ThreadSafeFunction` object.
- `[optional] callback`: The `Function` to call from another thread.
- `[optional] resource`: An object associated with the async work that will be
  passed to possible async_hooks init hooks.
- `resourceName`: A JavaScript string to provide an identifier for the kind of
  resource that is being provided for diagnostic information exposed by the
  async_hooks API.
- `maxQueueSize`: Maximum size of the queue. `0` for no limit.
- `initialThreadCount`: The initial number of threads, including the main
  thread, which will be making use of this function.
- `[optional] context`: Data to attach to the resulting `ThreadSafeFunction`. It
  can be retrieved via `GetContext()`.
- `[optional] finalizeCallback`: Function to call when the
  `TypedThreadSafeFunction` is being destroyed.  This callback will be invoked
  on the main thread when the thread-safe function is about to be destroyed. It
  receives the context and the finalize data given during construction (if
  given), and provides an opportunity for cleaning up after the threads e.g. by
  calling `uv_thread_join()`. It is important that, aside from the main loop
  thread, there be no threads left using the thread-safe function after the
  finalize callback completes. Must implement `void operator()(Env env,
  FinalizerDataType* data, ContextType* hint)`.
- `[optional] data`: Data to be passed to `finalizeCallback`.

Returns a non-empty `Napi::TypedThreadSafeFunction` instance.

Depending on the targeted `NAPI_VERSION`, the API has different implementations
for `CallbackType callback`.

When targeting version 4, `callback` may be:
- of type `const Function&`
- not provided as a parameter, in which case the API creates a new no-op
  `Function`

When targeting version 5+, `callback` may be:
- of type `const Function&`
- of type `std::nullptr_t`
- not provided as a parameter, in which case the API passes `std::nullptr`

### Acquire

Adds a thread to this thread-safe function object, indicating that a new thread
will start making use of the thread-safe function.

```cpp
napi_status Napi::TypedThreadSafeFunction<ContextType, DataType, Callback>::Acquire()
```

Returns one of:
- `napi_ok`: The thread has successfully acquired the thread-safe function for
  its use.
- `napi_closing`: The thread-safe function has been marked as closing via a
  previous call to `Abort()`.

### Release

Indicates that an existing thread will stop making use of the thread-safe
function. A thread should call this API when it stops making use of this
thread-safe function. Using any thread-safe APIs after having called this API
has undefined results in the current thread, as the thread-safe function may
have been destroyed.

```cpp
napi_status Napi::TypedThreadSafeFunction<ContextType, DataType, Callback>::Release() const
```

Returns one of:
- `napi_ok`: The thread-safe function has been successfully released.
- `napi_invalid_arg`: The thread-safe function's thread-count is zero.
- `napi_generic_failure`: A generic error occurred when attempting to release the
  thread-safe function.

### Abort

"Aborts" the thread-safe function. This will cause all subsequent APIs
associated with the thread-safe function except `Release()` to return
`napi_closing` even before its reference count reaches zero. In particular,
`BlockingCall` and `NonBlockingCall()` will return `napi_closing`, thus
informing the threads that it is no longer possible to make asynchronous calls
to the thread-safe function. This can be used as a criterion for terminating the
thread. Upon receiving a return value of `napi_closing` from a thread-safe
function call a thread must make no further use of the thread-safe function
because it is no longer guaranteed to be allocated.

```cpp
napi_status Napi::TypedThreadSafeFunction<ContextType, DataType, Callback>::Abort() const
```

Returns one of:
- `napi_ok`: The thread-safe function has been successfully aborted.
- `napi_invalid_arg`: The thread-safe function's thread-count is zero.
- `napi_generic_failure`: A generic error occurred when attempting to abort the
  thread-safe function.

### BlockingCall / NonBlockingCall

Calls the Javascript function in either a blocking or non-blocking fashion.
- `BlockingCall()`: the API blocks until space becomes available in the queue.
  Will never block if the thread-safe function was created with a maximum queue
  size of `0`.
- `NonBlockingCall()`: will return `napi_queue_full` if the queue was full,
  preventing data from being successfully added to the queue.

```cpp
napi_status Napi::TypedThreadSafeFunction<ContextType, DataType, Callback>::BlockingCall(DataType* data = nullptr) const

napi_status Napi::TypedThreadSafeFunction<ContextType, DataType, Callback>::NonBlockingCall(DataType* data = nullptr) const
```

- `[optional] data`: Data to pass to the callback which was passed to
  `TypedThreadSafeFunction::New()`.

Returns one of:
- `napi_ok`: `data` was successfully added to the queue.
- `napi_queue_full`: The queue was full when trying to call in a non-blocking
  method.
- `napi_closing`: The thread-safe function is aborted and no further calls can
  be made.
- `napi_invalid_arg`: The thread-safe function is closed.
- `napi_generic_failure`: A generic error occurred when attempting to add to the
  queue.


## Example

```cpp
#include <chrono>
#include <napi.h>
#include <thread>

using namespace Napi;

using Context = Reference<Value>;
using DataType = int;
void CallJs(Napi::Env env, Function callback, Context *context, DataType *data);
using TSFN = TypedThreadSafeFunction<Context, DataType, CallJs>;
using FinalizerDataType = void;

std::thread nativeThread;
TSFN tsfn;

Value Start(const CallbackInfo &info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    throw TypeError::New(env, "Expected two arguments");
  } else if (!info[0].IsFunction()) {
    throw TypeError::New(env, "Expected first arg to be function");
  } else if (!info[1].IsNumber()) {
    throw TypeError::New(env, "Expected second arg to be number");
  }

  int count = info[1].As<Number>().Int32Value();

  // Create a new context set to the receiver (ie, `this`) of the function call
  Context *context = new Reference<Value>(Persistent(info.This()));

  // Create a ThreadSafeFunction
  tsfn = TSFN::New(
      env,
      info[0].As<Function>(), // JavaScript function called asynchronously
      "Resource Name",        // Name
      0,                      // Unlimited queue
      1,                      // Only one thread will use this initially
      context,
      [](Napi::Env, FinalizerDataType *,
         Context *ctx) { // Finalizer used to clean threads up
        nativeThread.join();
        delete ctx;
      });

  // Create a native thread
  nativeThread = std::thread([count] {
    for (int i = 0; i < count; i++) {
      // Create new data
      int *value = new int(clock());

      // Perform a blocking call
      napi_status status = tsfn.BlockingCall(value);
      if (status != napi_ok) {
        // Handle error
        break;
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Release the thread-safe function
    tsfn.Release();
  });

  return Boolean::New(env, true);
}

// Transform native data into JS data, passing it to the provided
// `callback` -- the TSFN's JavaScript function.
void CallJs(Napi::Env env, Function callback, Context *context,
            DataType *data) {
  // Is the JavaScript environment still available to call into, eg. the TSFN is
  // not aborted
  if (env != nullptr) {
    // On Node-API 5+, the `callback` parameter is optional; however, this example
    // does ensure a callback is provided.
    if (callback != nullptr) {
      callback.Call(context->Value(), {Number::New(env, *data)});
    }
  }
  if (data != nullptr) {
    // We're finished with the data.
    delete data;
  }
}

Napi::Object Init(Napi::Env env, Object exports) {
  exports.Set("start", Function::New(env, Start));
  return exports;
}

NODE_API_MODULE(clock, Init)
```

The above code can be used from JavaScript as follows:

```js
const { start } = require('bindings')('clock');

start.call(new Date(), function (clock) {
    const context = this;
    console.log(context, clock);
}, 5);
```

When executed, the output will show the value of `clock()` five times at one
second intervals, prefixed with the TSFN's context -- `start`'s receiver (ie,
`new Date()`):

```
2020-08-18T21:04:25.116Z 49824
2020-08-18T21:04:25.116Z 62493
2020-08-18T21:04:25.116Z 62919
2020-08-18T21:04:25.116Z 63228
2020-08-18T21:04:25.116Z 63531
```
