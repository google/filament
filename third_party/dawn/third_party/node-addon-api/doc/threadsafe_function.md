# ThreadSafeFunction

The `Napi::ThreadSafeFunction` type provides APIs for threads to communicate
with the addon's main thread to invoke JavaScript functions on their behalf.
Documentation can be found for an [overview of the API](threadsafe.md), as well
as [differences between the two thread-safe function
APIs](threadsafe.md#implementation-differences).

## Methods

### Constructor

Creates a new empty instance of `Napi::ThreadSafeFunction`.

```cpp
Napi::Function::ThreadSafeFunction();
```

### Constructor

Creates a new instance of the `Napi::ThreadSafeFunction` object.

```cpp
Napi::ThreadSafeFunction::ThreadSafeFunction(napi_threadsafe_function tsfn);
```

- `tsfn`: The `napi_threadsafe_function` which is a handle for an existing
  thread-safe function.

Returns a non-empty `Napi::ThreadSafeFunction` instance. When using this
constructor, only use the `Blocking(void*)` / `NonBlocking(void*)` overloads;
the `Callback` and templated `data*` overloads should _not_ be used. See below
for additional details.

### New

Creates a new instance of the `Napi::ThreadSafeFunction` object. The `New`
function has several overloads for the various optional parameters: skip the
optional parameter for that specific overload.

```cpp
New(napi_env env,
    const Function& callback,
    const Object& resource,
    ResourceString resourceName,
    size_t maxQueueSize,
    size_t initialThreadCount,
    ContextType* context,
    Finalizer finalizeCallback,
    FinalizerDataType* data);
```

- `env`: The `napi_env` environment in which to construct the
  `Napi::ThreadSafeFunction` object.
- `callback`: The `Function` to call from another thread.
- `[optional] resource`: An object associated with the async work that will be
  passed to possible async_hooks init hooks.
- `resourceName`: A JavaScript string to provide an identifier for the kind of
  resource that is being provided for diagnostic information exposed by the
  async_hooks API.
- `maxQueueSize`: Maximum size of the queue. `0` for no limit.
- `initialThreadCount`: The initial number of threads, including the main
  thread, which will be making use of this function.
- `[optional] context`: Data to attach to the resulting `ThreadSafeFunction`. It
  can be retreived by calling `GetContext()`.
- `[optional] finalizeCallback`: Function to call when the `ThreadSafeFunction`
  is being destroyed.  This callback will be invoked on the main thread when the
  thread-safe function is about to be destroyed. It receives the context and the
  finalize data given during construction (if given), and provides an
  opportunity for cleaning up after the threads e.g. by calling
  `uv_thread_join()`. It is important that, aside from the main loop thread,
  there be no threads left using the thread-safe function after the finalize
  callback completes. Must implement `void operator()(Env env, DataType* data,
  ContextType* hint)`, skipping `data` or `hint` if they are not provided. Can
  be retrieved via `GetContext()`.
- `[optional] data`: Data to be passed to `finalizeCallback`.

Returns a non-empty `Napi::ThreadSafeFunction` instance.

### Acquire

Add a thread to this thread-safe function object, indicating that a new thread
will start making use of the thread-safe function.

```cpp
napi_status Napi::ThreadSafeFunction::Acquire() const
```

Returns one of:
- `napi_ok`: The thread has successfully acquired the thread-safe function
for its use.
- `napi_closing`: The thread-safe function has been marked as closing via a
previous call to `Abort()`.

### Release

Indicate that an existing thread will stop making use of the thread-safe
function. A thread should call this API when it stops making use of this
thread-safe function. Using any thread-safe APIs after having called this API
has undefined results in the current thread, as it may have been destroyed.

```cpp
napi_status Napi::ThreadSafeFunction::Release() const
```

Returns one of:
- `napi_ok`: The thread-safe function has been successfully released.
- `napi_invalid_arg`: The thread-safe function's thread-count is zero.
- `napi_generic_failure`: A generic error occurred when attempting to release
the thread-safe function.

### Abort

"Abort" the thread-safe function. This will cause all subsequent APIs associated
with the thread-safe function except `Release()` to return `napi_closing` even
before its reference count reaches zero. In particular, `BlockingCall` and
`NonBlockingCall()` will return `napi_closing`, thus informing the threads that
it is no longer possible to make asynchronous calls to the thread-safe function.
This can be used as a criterion for terminating the thread. Upon receiving a
return value of `napi_closing` from a thread-safe function call a thread must
make no further use of the thread-safe function because it is no longer
guaranteed to be allocated.

```cpp
napi_status Napi::ThreadSafeFunction::Abort() const
```

Returns one of:
- `napi_ok`: The thread-safe function has been successfully aborted.
- `napi_invalid_arg`: The thread-safe function's thread-count is zero.
- `napi_generic_failure`: A generic error occurred when attempting to abort
the thread-safe function.

### BlockingCall / NonBlockingCall

Calls the Javascript function in either a blocking or non-blocking fashion.
- `BlockingCall()`: the API blocks until space becomes available in the queue.
  Will never block if the thread-safe function was created with a maximum queue
  size of `0`.
- `NonBlockingCall()`: will return `napi_queue_full` if the queue was full,
  preventing data from being successfully added to the queue.

There are several overloaded implementations of `BlockingCall()` and
`NonBlockingCall()` for use with optional parameters: skip the optional
parameter for that specific overload.

**These specific function overloads should only be used on a `ThreadSafeFunction`
created via `ThreadSafeFunction::New`.**

```cpp
napi_status Napi::ThreadSafeFunction::BlockingCall(DataType* data, Callback callback) const

napi_status Napi::ThreadSafeFunction::NonBlockingCall(DataType* data, Callback callback) const
```

- `[optional] data`: Data to pass to `callback`.
- `[optional] callback`: C++ function that is invoked on the main thread. The
  callback receives the `ThreadSafeFunction`'s JavaScript callback function to
  call as an `Napi::Function` in its parameters and the `DataType*` data pointer
  (if provided). Must implement `void operator()(Napi::Env env, Function
  jsCallback, DataType* data)`, skipping `data` if not provided. It is not
  necessary to call into JavaScript via `MakeCallback()` because Node-API runs
  `callback` in a context appropriate for callbacks.

**These specific function overloads should only be used on a `ThreadSafeFunction`
created via `ThreadSafeFunction(napi_threadsafe_function)`.**

```cpp
napi_status Napi::ThreadSafeFunction::BlockingCall(void* data) const

napi_status Napi::ThreadSafeFunction::NonBlockingCall(void* data) const
```
- `data`: Data to pass to `call_js_cb` specified when creating the thread-safe
  function via `napi_create_threadsafe_function`.

Returns one of:
- `napi_ok`: The call was successfully added to the queue.
- `napi_queue_full`: The queue was full when trying to call in a non-blocking
  method.
- `napi_closing`: The thread-safe function is aborted and cannot accept more
  calls.
- `napi_invalid_arg`: The thread-safe function is closed.
- `napi_generic_failure`: A generic error occurred when attempting to add to the
  queue.

## Example

```cpp
#include <chrono>
#include <thread>
#include <napi.h>

using namespace Napi;

std::thread nativeThread;
ThreadSafeFunction tsfn;

Value Start( const CallbackInfo& info )
{
  Napi::Env env = info.Env();

  if ( info.Length() < 2 )
  {
    throw TypeError::New( env, "Expected two arguments" );
  }
  else if ( !info[0].IsFunction() )
  {
    throw TypeError::New( env, "Expected first arg to be function" );
  }
  else if ( !info[1].IsNumber() )
  {
    throw TypeError::New( env, "Expected second arg to be number" );
  }

  int count = info[1].As<Number>().Int32Value();

  // Create a ThreadSafeFunction
  tsfn = ThreadSafeFunction::New(
      env,
      info[0].As<Function>(),  // JavaScript function called asynchronously
      "Resource Name",         // Name
      0,                       // Unlimited queue
      1,                       // Only one thread will use this initially
      []( Napi::Env ) {        // Finalizer used to clean threads up
        nativeThread.join();
      } );

  // Create a native thread
  nativeThread = std::thread( [count] {
    auto callback = []( Napi::Env env, Function jsCallback, int* value ) {
      // Transform native data into JS data, passing it to the provided
      // `jsCallback` -- the TSFN's JavaScript function.
      jsCallback.Call( {Number::New( env, *value )} );

      // We're finished with the data.
      delete value;
    };

    for ( int i = 0; i < count; i++ )
    {
      // Create new data
      int* value = new int( clock() );

      // Perform a blocking call
      napi_status status = tsfn.BlockingCall( value, callback );
      if ( status != napi_ok )
      {
        // Handle error
        break;
      }

      std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
    }

    // Release the thread-safe function
    tsfn.Release();
  } );

  return Boolean::New(env, true);
}

Napi::Object Init( Napi::Env env, Object exports )
{
  exports.Set( "start", Function::New( env, Start ) );
  return exports;
}

NODE_API_MODULE( clock, Init )
```

The above code can be used from JavaScript as follows:

```js
const { start } = require('bindings')('clock');

start(function () {
    console.log("JavaScript callback called with arguments", Array.from(arguments));
}, 5);
```

When executed, the output will show the value of `clock()` five times at one
second intervals:

```
JavaScript callback called with arguments [ 84745 ]
JavaScript callback called with arguments [ 103211 ]
JavaScript callback called with arguments [ 104516 ]
JavaScript callback called with arguments [ 105104 ]
JavaScript callback called with arguments [ 105691 ]
```
