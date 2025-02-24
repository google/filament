# AsyncProgressWorker

`Napi::AsyncProgressWorker` is an abstract class which implements `Napi::AsyncWorker`
while extending `Napi::AsyncWorker` internally with `Napi::ThreadSafeFunction` for
moving work progress reports from worker thread(s) to event loop threads.

Like `Napi::AsyncWorker`, once created, execution is requested by calling
`Napi::AsyncProgressWorker::Queue`. When a thread is available for execution
the `Napi::AsyncProgressWorker::Execute` method will be invoked. During the
execution, `Napi::AsyncProgressWorker::ExecutionProgress::Send` can be used to
indicate execution process, which will eventually invoke `Napi::AsyncProgressWorker::OnProgress`
on the JavaScript thread to safely call into JavaScript. Once `Napi::AsyncProgressWorker::Execute`
completes either `Napi::AsyncProgressWorker::OnOK` or `Napi::AsyncProgressWorker::OnError`
will be invoked. Once the `Napi::AsyncProgressWorker::OnOK` or `Napi::AsyncProgressWorker::OnError`
methods are complete the `Napi::AsyncProgressWorker` instance is destructed.

For the most basic use, only the `Napi::AsyncProgressWorker::Execute` and
`Napi::AsyncProgressWorker::OnProgress` method must be implemented in a subclass.

## Methods

[`Napi::AsyncWorker`][] provides detailed descriptions for most methods.

### Execute

This method is used to execute some tasks outside of the **event loop** on a libuv
worker thread. Subclasses must implement this method and the method is run on
a thread other than that running the main event loop. As the method is not
running on the main event loop, it must avoid calling any methods from node-addon-api
or running any code that might invoke JavaScript. Instead, once this method is
complete any interaction through node-addon-api with JavaScript should be implemented
in the `Napi::AsyncProgressWorker::OnOK` method and/or `Napi::AsyncProgressWorker::OnError`
which run on the main thread and are invoked when the `Napi::AsyncProgressWorker::Execute`
method completes.

```cpp
virtual void Napi::AsyncProgressWorker::Execute(const ExecutionProgress& progress) = 0;
```

### OnOK

This method is invoked when the computation in the `Execute` method ends.
The default implementation runs the `Callback` optionally provided when the
`AsyncProgressWorker` class was created. The `Callback` will by default receive no
arguments. Arguments to the callback can be provided by overriding the `GetResult()`
method.

```cpp
virtual void Napi::AsyncProgressWorker::OnOK();
```

### OnProgress

This method is invoked when the computation in the
`Napi::AsyncProgressWorker::ExecutionProgress::Send` method was called during
worker thread execution. This method can also be triggered via a call to
`Napi::AsyncProgress[Queue]Worker::ExecutionProgress::Signal`, in which case the
`data` parameter will be `nullptr`.

```cpp
virtual void Napi::AsyncProgressWorker::OnProgress(const T* data, size_t count)
```

### Constructor

Creates a new `Napi::AsyncProgressWorker`.

```cpp
explicit Napi::AsyncProgressWorker(const Napi::Function& callback);
```

- `[in] callback`: The function which will be called when an asynchronous
operations ends. The given function is called from the main event loop thread.

Returns a `Napi::AsyncProgressWorker` instance which can later be queued for execution by
calling `Napi::AsyncWork::Queue`.

### Constructor

Creates a new `Napi::AsyncProgressWorker`.

```cpp
explicit Napi::AsyncProgressWorker(const Napi::Function& callback, const char* resource_name);
```

- `[in] callback`: The function which will be called when an asynchronous
operations ends. The given function is called from the main event loop thread.
- `[in] resource_name`: Null-terminated string that represents the
identifier for the kind of resource that is being provided for diagnostic
information exposed by the async_hooks API.

Returns a `Napi::AsyncProgressWorker` instance which can later be queued for execution by
calling `Napi::AsyncWork::Queue`.

### Constructor

Creates a new `Napi::AsyncProgressWorker`.

```cpp
explicit Napi::AsyncProgressWorker(const Napi::Function& callback, const char* resource_name, const Napi::Object& resource);
```

- `[in] callback`: The function which will be called when an asynchronous
operations ends. The given function is called from the main event loop thread.
- `[in] resource_name`:  Null-terminated string that represents the
identifier for the kind of resource that is being provided for diagnostic
information exposed by the async_hooks API.
- `[in] resource`: Object associated with the asynchronous operation that
will be passed to possible async_hooks.

Returns a `Napi::AsyncProgressWorker` instance which can later be queued for execution by
calling `Napi::AsyncWork::Queue`.

### Constructor

Creates a new `Napi::AsyncProgressWorker`.

```cpp
explicit Napi::AsyncProgressWorker(const Napi::Object& receiver, const Napi::Function& callback);
```

- `[in] receiver`: The `this` object passed to the called function.
- `[in] callback`: The function which will be called when an asynchronous
operations ends. The given function is called from the main event loop thread.

Returns a `Napi::AsyncProgressWorker` instance which can later be queued for execution by
calling `Napi::AsyncWork::Queue`.

### Constructor

Creates a new `Napi::AsyncProgressWorker`.

```cpp
explicit Napi::AsyncProgressWorker(const Napi::Object& receiver, const Napi::Function& callback, const char* resource_name);
```

- `[in] receiver`: The `this` object passed to the called function.
- `[in] callback`: The function which will be called when an asynchronous
operations ends. The given function is called from the main event loop thread.
- `[in] resource_name`:  Null-terminated string that represents the
identifier for the kind of resource that is being provided for diagnostic
information exposed by the async_hooks API.

Returns a `Napi::AsyncWork` instance which can later be queued for execution by
calling `Napi::AsyncWork::Queue`.

### Constructor

Creates a new `Napi::AsyncProgressWorker`.

```cpp
explicit Napi::AsyncProgressWorker(const Napi::Object& receiver, const Napi::Function& callback, const char* resource_name, const Napi::Object& resource);
```

- `[in] receiver`: The `this` object to be passed to the called function.
- `[in] callback`: The function which will be called when an asynchronous
operations ends. The given function is called from the main event loop thread.
- `[in] resource_name`:  Null-terminated string that represents the
identifier for the kind of resource that is being provided for diagnostic
information exposed by the async_hooks API.
- `[in] resource`: Object associated with the asynchronous operation that
will be passed to possible async_hooks.

Returns a `Napi::AsyncWork` instance which can later be queued for execution by
calling `Napi::AsyncWork::Queue`.

### Constructor

Creates a new `Napi::AsyncProgressWorker`.

```cpp
explicit Napi::AsyncProgressWorker(Napi::Env env);
```

- `[in] env`: The environment in which to create the `Napi::AsyncProgressWorker`.

Returns an `Napi::AsyncProgressWorker` instance which can later be queued for execution by calling
`Napi::AsyncProgressWorker::Queue`.

Available with `NAPI_VERSION` equal to or greater than 5.

### Constructor

Creates a new `Napi::AsyncProgressWorker`.

```cpp
explicit Napi::AsyncProgressWorker(Napi::Env env, const char* resource_name);
```

- `[in] env`: The environment in which to create the `Napi::AsyncProgressWorker`.
- `[in] resource_name`: Null-terminated string that represents the
identifier for the kind of resource that is being provided for diagnostic
information exposed by the async_hooks API.

Returns a `Napi::AsyncProgressWorker` instance which can later be queued for execution by
calling `Napi::AsyncProgressWorker::Queue`.

Available with `NAPI_VERSION` equal to or greater than 5.

### Constructor

Creates a new `Napi::AsyncProgressWorker`.

```cpp
explicit Napi::AsyncProgressWorker(Napi::Env env, const char* resource_name, const Napi::Object& resource);
```

- `[in] env`: The environment in which to create the `Napi::AsyncProgressWorker`.
- `[in] resource_name`:  Null-terminated string that represents the
identifier for the kind of resource that is being provided for diagnostic
information exposed by the async_hooks API.
- `[in] resource`: Object associated with the asynchronous operation that
will be passed to possible async_hooks.

Returns a `Napi::AsyncProgressWorker` instance which can later be queued for execution by
calling `Napi::AsyncProgressWorker::Queue`.

Available with `NAPI_VERSION` equal to or greater than 5.

### Destructor

Deletes the created work object that is used to execute logic asynchronously and
release the internal `Napi::ThreadSafeFunction`, which will be aborted to prevent
unexpected upcoming thread safe calls.

```cpp
virtual Napi::AsyncProgressWorker::~AsyncProgressWorker();
```

# AsyncProgressWorker::ExecutionProgress

A bridge class created before the worker thread execution of `Napi::AsyncProgressWorker::Execute`.

## Methods

### Send

`Napi::AsyncProgressWorker::ExecutionProgress::Send` takes two arguments, a pointer
to a generic type of data, and a `size_t` to indicate how many items the pointer is
pointing to.

The data pointed to will be copied to internal slots of `Napi::AsyncProgressWorker` so
after the call to `Napi::AsyncProgressWorker::ExecutionProgress::Send` the data can
be safely released.

Note that `Napi::AsyncProgressWorker::ExecutionProgress::Send` merely guarantees
**eventual** invocation of `Napi::AsyncProgressWorker::OnProgress`, which means
multiple send might be coalesced into single invocation of `Napi::AsyncProgressWorker::OnProgress`
with latest data. If you would like to guarantee that there is one invocation of
`OnProgress` for every `Send` call, you should use the `Napi::AsyncProgressQueueWorker` 
class instead which is documented further down this page.

```cpp
void Napi::AsyncProgressWorker::ExecutionProgress::Send(const T* data, size_t count) const;
```

### Signal

`Napi::AsyncProgressWorker::ExecutionProgress::Signal` triggers an invocation of
`Napi::AsyncProgressWorker::OnProgress` with `nullptr` as the `data` parameter.

```cpp
void Napi::AsyncProgressWorker::ExecutionProgress::Signal();
```

## Example

The first step to use the `Napi::AsyncProgressWorker` class is to create a new class that
inherits from it and implement the `Napi::AsyncProgressWorker::Execute` abstract method.
Typically input to the worker will be saved within the class' fields generally
passed in through its constructor.

During the worker thread execution, the first argument of `Napi::AsyncProgressWorker::Execute`
can be used to report the progress of the execution.

When the `Napi::AsyncProgressWorker::Execute` method completes without errors the
`Napi::AsyncProgressWorker::OnOK` function callback will be invoked. In this function the
results of the computation will be reassembled and returned back to the initial
JavaScript context.

`Napi::AsyncProgressWorker` ensures that all the code in the `Napi::AsyncProgressWorker::Execute`
function runs in the background out of the **event loop** thread and at the end
the `Napi::AsyncProgressWorker::OnOK` or `Napi::AsyncProgressWorker::OnError` function will be
called and are executed as part of the event loop.

The code below shows a basic example of the `Napi::AsyncProgressWorker` implementation along with an
example of how the counterpart in Javascript would appear:

```cpp
#include <napi.h>

#include <chrono>
#include <thread>

using namespace Napi;

class EchoWorker : public AsyncProgressWorker<uint32_t> {
    public:
        EchoWorker(Function& okCallback, std::string& echo)
        : AsyncProgressWorker(okCallback), echo(echo) {}

        ~EchoWorker() {}
        
        // This code will be executed on the worker thread
        void Execute(const ExecutionProgress& progress) {
            // Need to simulate cpu heavy task
            // Note: This Send() call is not guaranteed to trigger an equal
            // number of OnProgress calls (read documentation above for more info)
            for (uint32_t i = 0; i < 100; ++i) {
              progress.Send(&i, 1)
            }
        }
        
        void OnError(const Error &e) {
            HandleScope scope(Env());
            // Pass error onto JS, no data for other parameters
            Callback().Call({String::New(Env(), e.Message())});
        }

        void OnOK() {
            HandleScope scope(Env());
            // Pass no error, give back original data
            Callback().Call({Env().Null(), String::New(Env(), echo)});
        }

        void OnProgress(const uint32_t* data, size_t /* count */) {
            HandleScope scope(Env());
            // Pass no error, no echo data, but do pass on the progress data
            Callback().Call({Env().Null(), Env().Null(), Number::New(Env(), *data)});
        }

    private:
        std::string echo;
};
```

The `EchoWorker`'s constructor calls the base class' constructor to pass in the
callback that the `Napi::AsyncProgressWorker` base class will store persistently. When
the work on the `Napi::AsyncProgressWorker::Execute` method is done the
`Napi::AsyncProgressWorker::OnOk` method is called and the results are return back to
JavaScript when the stored callback is invoked with its associated environment.

The following code shows an example of how to create and use an `Napi::AsyncProgressWorker`

```cpp
#include <napi.h>

// Include EchoWorker class
// ..

using namespace Napi;

Value Echo(const CallbackInfo& info) {
    // We need to validate the arguments here
    std::string in = info[0].As<String>();
    Function cb = info[1].As<Function>();
    EchoWorker* wk = new EchoWorker(cb, in);
    wk->Queue();
    return info.Env().Undefined();
}

// Register the native method for JS to access
Object Init(Env env, Object exports)
{
    exports.Set(String::New(env, "echo"), Function::New(env, Echo));

    return exports;
}

// Register our native addon
NODE_API_MODULE(nativeAddon, Init)
```

The implementation of a `Napi::AsyncProgressWorker` can be used by creating a
new instance and passing to its constructor the callback to execute when the
asynchronous task ends and other data needed for the computation. Once created,
the only other action needed is to call the `Napi::AsyncProgressWorker::Queue`
method that will queue the created worker for execution.

Lastly, the following Javascript (ES6+) code would be associated the above example:

```js
const { nativeAddon } = require('binding.node');

const exampleCallback = (errorResponse, okResponse, progressData) => {
    // Use the data accordingly
    // ...
};

// Call our native addon with the parameters of a string and a function
nativeAddon.echo("example", exampleCallback);
```

# AsyncProgressQueueWorker

`Napi::AsyncProgressQueueWorker` acts exactly like `Napi::AsyncProgressWorker`
except that each progress committed by `Napi::AsyncProgressQueueWorker::ExecutionProgress::Send`
during `Napi::AsyncProgressQueueWorker::Execute` is guaranteed to be
processed by `Napi::AsyncProgressQueueWorker::OnProgress` on the JavaScript
thread in the order it was committed.

For the most basic use, only the `Napi::AsyncProgressQueueWorker::Execute` and
`Napi::AsyncProgressQueueWorker::OnProgress` method must be implemented in a subclass.

# AsyncProgressQueueWorker::ExecutionProgress

A bridge class created before the worker thread execution of `Napi::AsyncProgressQueueWorker::Execute`.

## Methods

### Send

`Napi::AsyncProgressQueueWorker::ExecutionProgress::Send` takes two arguments, a pointer
to a generic type of data, and a `size_t` to indicate how many items the pointer is
pointing to.

The data pointed to will be copied to internal slots of `Napi::AsyncProgressQueueWorker` so
after the call to `Napi::AsyncProgressQueueWorker::ExecutionProgress::Send` the data can
be safely released.

`Napi::AsyncProgressQueueWorker::ExecutionProgress::Send` guarantees invocation
of `Napi::AsyncProgressQueueWorker::OnProgress`, which means multiple `Send`
call will result in the in-order invocation of `Napi::AsyncProgressQueueWorker::OnProgress`
with each data item.

```cpp
void Napi::AsyncProgressQueueWorker::ExecutionProgress::Send(const T* data, size_t count) const;
```

### Signal

`Napi::AsyncProgressQueueWorker::ExecutionProgress::Signal` triggers an invocation of
`Napi::AsyncProgressQueueWorker::OnProgress` with `nullptr` as the `data` parameter.

```cpp
void Napi::AsyncProgressQueueWorker::ExecutionProgress::Signal() const;
```

## Example

The code below shows an example of the `Napi::AsyncProgressQueueWorker` implementation, but
also demonstrates how to use multiple `Napi::Function`'s if you wish to provide multiple
callback functions for more object-oriented code:

```cpp
#include <napi.h>

#include <chrono>
#include <thread>

using namespace Napi;

class EchoWorker : public AsyncProgressQueueWorker<uint32_t> {
    public:
        EchoWorker(Function& okCallback, Function& errorCallback, Function& progressCallback, std::string& echo)
        : AsyncProgressQueueWorker(okCallback), echo(echo) {
            // Set our function references to use them below
            this->errorCallback.Reset(errorCallback, 1);
            this->progressCallback.Reset(progressCallback, 1);
        }

        ~EchoWorker() {}
        
        // This code will be executed on the worker thread
        void Execute(const ExecutionProgress& progress) {
            // Need to simulate cpu heavy task to demonstrate that
            // every call to Send() will trigger an OnProgress function call
            for (uint32_t i = 0; i < 100; ++i) {
              progress.Send(&i, 1);
            }
        }

        void OnOK() {
            HandleScope scope(Env());
            // Call our onOkCallback in javascript with the data we were given originally
            Callback().Call({String::New(Env(), echo)});
        }
        
        void OnError(const Error &e) {
            HandleScope scope(Env());
            
            // We call our callback provided in the constructor with 2 parameters
            if (!this->errorCallback.IsEmpty()) {
                // Call our onErrorCallback in javascript with the error message
                this->errorCallback.Call(Receiver().Value(), {String::New(Env(), e.Message())});
            }
        }

        void OnProgress(const uint32_t* data, size_t /* count */) {
            HandleScope scope(Env());
            
            if (!this->progressCallback.IsEmpty()) {
                // Call our onProgressCallback in javascript with each integer from 0 to 99 (inclusive)
                // as this function is triggered from the above Send() calls
                this->progressCallback.Call(Receiver().Value(), {Number::New(Env(), *data)});
            }
        }

    private:
        std::string echo;
        FunctionReference progressCallback;
        FunctionReference errorCallback;
        
};
```

The `EchoWorker`'s constructor calls the base class' constructor to pass in the
callback that the `Napi::AsyncProgressQueueWorker` base class will store
persistently. When the work on the `Napi::AsyncProgressQueueWorker::Execute`
method is done the `Napi::AsyncProgressQueueWorker::OnOk` method is called and
the results are returned back to JavaScript when the stored callback is invoked
with its associated environment.

The following code shows an example of how to create and use an
`Napi::AsyncProgressQueueWorker`.

```cpp
#include <napi.h>

// Include EchoWorker class
// ..

using namespace Napi;

Value Echo(const CallbackInfo& info) {
    // We need to validate the arguments here.
    std::string in = info[0].As<String>();
    Function errorCb = info[1].As<Function>();
    Function okCb = info[2].As<Function>();
    Function progressCb = info[3].As<Function>();
    EchoWorker* wk = new EchoWorker(okCb, errorCb, progressCb, in);
    wk->Queue();
    return info.Env().Undefined();
}

// Register the native method for JS to access
Object Init(Env env, Object exports)
{
    exports.Set(String::New(env, "echo"), Function::New(env, Echo));

    return exports;
}

// Register our native addon
NODE_API_MODULE(nativeAddon, Init)
```

The implementation of a `Napi::AsyncProgressQueueWorker` can be used by creating a
new instance and passing to its constructor the callback to execute when the
asynchronous task ends and other data needed for the computation. Once created,
the only other action needed is to call the `Napi::AsyncProgressQueueWorker::Queue`
method that will queue the created worker for execution.

Lastly, the following Javascript (ES6+) code would be associated the above example:

```js
const { nativeAddon } = require('binding.node');

const onErrorCallback = (msg) => {
    // Use the data accordingly
    // ...
};

const onOkCallback = (echo) => {
    // Use the data accordingly
    // ...
};

const onProgressCallback = (num) => {
    // Use the data accordingly
    // ...
};

// Call our native addon with the parameters of a string and three callback functions
nativeAddon.echo("example", onErrorCallback, onOkCallback, onProgressCallback);
```

[`Napi::AsyncWorker`]: ./async_worker.md
