# Errors {#Errors}

Errors are surfaced in several ways.

Most errors only result from incorrect use of the API and should not need to be handled at runtime.
However, a few (@ref WGPUErrorType_OutOfMemory, @ref WGPUErrorType_Internal) are potentially useful to handle.

## Device Error {#DeviceError}

These behave the same way as [in the WebGPU JavaScript API specification](https://www.w3.org/TR/webgpu/#errors-and-debugging).
They are receivable via @ref wgpuDevicePopErrorScope() and @ref WGPUDeviceDescriptor::uncapturedErrorCallbackInfo.

These errors include:

- All device-timeline errors in the WebGPU specification.
- Enum values which are numerically invalid (this is not possible in JavaScript).
- Enum values which are require features not enabled on the device (a [content-timeline](https://www.w3.org/TR/webgpu/#content-timeline) error in JavaScript), for example compressed texture formats.
- Other content-timeline errors where specified.
- @ref NonFiniteFloatValueError (except when they interrupt Wasm execution)

### Error Scopes {#ErrorScopes}

Error scopes are used via @ref wgpuDevicePushErrorScope, @ref wgpuDevicePopErrorScope, and @ref WGPUDeviceDescriptor::uncapturedErrorCallbackInfo. These behave the same as in the JavaScript API, except for considerations around multi-threading (which JavaScript doesn't have at the time of this writing):

- The error scope stack state is **thread-local**: each thread has a separate stack, which is initially empty. The error scope that captures an error depends on which thread made the API call that generated the error.
    Note in particular:
    - Async callbacks run on various threads and with unpredictable timing (see @ref Asynchronous-Operations), except when using @ref wgpuInstanceWaitAny. To avoid race conditions, if error scopes are used, applications generally should avoid having device errors escape from an async function, and/or should not keep scopes open when callbacks that could produce errors may run.
    - Runtimes with async task support (I/O runtimes, language async/coroutines/futures, etc.) may use "green threads" style systems to schedule tasks on different OS threads. Error scope stack state is OS-thread-local, not green-thread-local.
- The UncapturedError callback (@ref WGPUDeviceDescriptor::uncapturedErrorCallbackInfo) is called for uncaptured errors on all threads. It **may** be called inline inside of a call to the API.

## Callback Error {#CallbackError}

These behave similarly to the Promise-returning JavaScript APIs. Instead of there being two callbacks like in JavaScript (one for resolve and one for reject), there is a single callback which receives a status code, and depending on the status, _either_ a valid result with an empty message string (`{NULL, 0}`), _or_ an invalid result with a non-empty message string.

## Synchronous Error {#SynchronousError}

These errors include:

- @ref OutStructChainError cases.
- [Content-timeline](https://www.w3.org/TR/webgpu/#content-timeline) errors other than those which are surfaced as @ref DeviceError in `webgpu.h`. See specific documentation to determine how each error is exposed.

Generally these will return some kind of failure status (like \ref WGPUStatus_Error) or `NULL`, and produce an @ref ImplementationDefinedLogging message.

### Implementation-Defined Logging {#ImplementationDefinedLogging}

Entry points may also specify that they produce "implementation-defined logging".
These messages are logged in an implementation defined way (e.g. to an implementation-specific callback, or to a logging runtime).
They are intended to be intended to be read by humans, useful primarily for development and crash reporting.
