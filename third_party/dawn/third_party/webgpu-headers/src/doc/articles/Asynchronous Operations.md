# Asynchronous Operations {#Asynchronous-Operations}

Asynchronous operations in webgpu.h return a @ref WGPUFuture. This is an opaque handle which applications may use to poll or wait for completion of the asynchronous operation.

On all implementations of webgpu.h, it must be possible to react to async operations without undue latency or spin-waiting.

## Creation

All asynchronous operations start when the application calls an asynchronous webgpu.h function. These functions return `WGPUFuture`, and take in their arguments a `CallbackInfo` struct like @ref WGPURequestDeviceCallbackInfo or @ref WGPUBufferMapCallbackInfo. `CallbackInfo` structs have the following members:
 - a `WGPUChainedStruct const * nextInChain` pointer, for extensions.
 - a @ref WGPUCallbackMode `mode` enum.
 - a `callback` function pointer. For example:
   ```c
   typedef void (*WGPUBufferMapCallback)(
       WGPUMapAsyncStatus status,
       WGPUStringView message,
       WGPU_NULLABLE void* userdata1,
       WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;
   ```
 - Two application-owned userdata members.<br>
   `void* userdata1`<br>
   `void* userdata2`

The `callback` function pointer is called when the application _observes completion_ of the asynchronous operation. The `userdata1` and `userdata2` members are passed back to the application as the last two arguments in the callback function. Callbacks **might not** be called unless the application explicitly flushes them in order to _observe completion_. The point in time a callback is called depends on the @ref WGPUCallbackMode of the operation. webgpu.h provides three callback modes: @ref WGPUCallbackMode_WaitAnyOnly, @ref WGPUCallbackMode_AllowProcessEvents, and @ref WGPUCallbackMode_AllowSpontaneous.

@ref WGPUCallbackMode_WaitAnyOnly

> @copydoc ::WGPUCallbackMode_WaitAnyOnly

@ref WGPUCallbackMode_AllowProcessEvents

> @copydoc ::WGPUCallbackMode_AllowProcessEvents

@ref WGPUCallbackMode_AllowSpontaneous

> @copydoc ::WGPUCallbackMode_AllowSpontaneous

## Callback Statuses {#CallbackStatuses}

Each event's callback has its own status enum. Generally, there will be one
success case, and several failure cases.

Every callback status enum has a `CallbackCancelled` status. This indicates that the callback was cancelled, though any background processing related to the event, like pipeline compilation, may not be complete.

`CallbackCancelled` happens when the last (external) reference to the @ref WGPUInstance is dropped.
At this point it becomes impossible to call @ref wgpuInstanceWaitAny or @ref wgpuInstanceProcessEvents.

## wgpuInstanceWaitAny {#Wait-Any}

`WGPUWaitStatus wgpuInstanceWaitAny(WGPUInstance, size_t futureCount, WGPUFutureWaitInfo * futures, uint64_t timeoutNS)`

Waits on any WGPUFuture in the list of `futures` to complete for `timeoutNS` nanoseconds. Returns when at least one `WGPUFuture` is completed or `timeoutNS` elapses, whichever is first. If `timeoutNS` is zero, all `futures` are polled once, without blocking.

- Returns @ref WGPUWaitStatus_Success if at least one `WGPUFuture` completes. Each future which _just_ completed has its respective callback fired. Each future which _is_ complete has its corresponding @ref WGPUFutureWaitInfo::completed set to true.
    - Note that the timeout applies only to the "wait" phase, and does not affect the callback firing phase.
- Returns @ref WGPUWaitStatus_TimedOut if the timeout passed without any futures completing.
- Returns @ref WGPUWaitStatus_Error if the call was invalid for any of the following reasons:
    - A @ref Timed-Wait was performed when @ref WGPUInstanceFeatureName_TimedWaitAny is not enabled.
    - The number of futures waited on in a @ref Timed-Wait is greater than the enabled WGPUInstanceFeatures::timedWaitAnyMaxCount.
    - A wait was attempted with @ref Mixed-Sources.

### Timed Wait {#Timed-Wait}

Use of _timed waits_ (`timeoutNS > 0`), must be enabled on the WGPUInstance in @ref wgpuCreateInstance() with @ref WGPUInstanceFeatureName_TimedWaitAny, and the number of futures waited on must be less than or equal to @ref WGPUInstanceLimits::timedWaitAnyMaxCount.

### Mixed Sources {#Mixed-Sources}

Asynchronous operations may originate from different sources. There are CPU-timeline operations and there are Queue-timeline operations. Within a _timed wait_, it is only valid to wait on `WGPUFuture`s originating from a single `WGPUQueue`. Waiting on two futures from different queues, or waiting on a Queue-timeline future and some other CPU-timeline future is an error.

#### CPU-Timeline Operations

- ::wgpuInstanceRequestAdapter
- ::wgpuAdapterRequestDevice
- ::wgpuShaderModuleGetCompilationInfo
- ::wgpuDeviceCreateRenderPipelineAsync
- ::wgpuDeviceCreateComputePipelineAsync
- ::wgpuDevicePopErrorScope

#### Queue-Timeline Operations

- ::wgpuBufferMapAsync
- ::wgpuQueueOnSubmittedWorkDone

## wgpuInstanceProcessEvents {#Process-Events}
`void wgpuInstanceProcessEvents(WGPUInstance)`

Processes asynchronous events on this `WGPUInstance`, calling any callbacks for asynchronous operations created with @ref WGPUCallbackMode_AllowProcessEvents that have completed. This is a non-blocking operation.

## Device Events

Device events are slightly different in that their callback info are passed on the `WGPUDeviceDescriptor`, instead of in a function argument.

- The DeviceLost callback is set via @ref WGPUDeviceDescriptor::deviceLostCallbackInfo.
  The Future for that event is @ref wgpuDeviceGetLostFuture.
- The UncapturedError callback is set via @ref WGPUDeviceDescriptor::uncapturedErrorCallbackInfo.
  It is a repeating event, not a future, and it does not have a callback mode (see docs).

The uncaptured error callback is guaranteed not to fire after the device becomes lost. When the device is lost, it is an appropriate time for the application to free userdata variables for the uncaptured error callback. Note that the device becomes lost _before_ the actual device lost callback fires. First the device state transitions to lost, then the device lost callback fires. The timing of the callback depends on the device lost callback mode.

## Callback Reentrancy {#CallbackReentrancy}

There are three cases of async/callback re-entrancy:

- Calls to the API are nested inside non-spontaneous callbacks (those with @ref WGPUCallbackMode_WaitAnyOnly or @ref WGPUCallbackMode_AllowProcessEvents). Such callbacks always happen in @ref wgpuInstanceWaitAny() or @ref wgpuInstanceProcessEvents().
    - This does not introduce any unsafety. Implementations are required to exit critical sections before invoking callbacks.
    - This includes making nested calls to @ref wgpuInstanceWaitAny() and @ref wgpuInstanceProcessEvents(). (However, beware of overflowing the call stack!)
- Calls to the API are nested inside @ref WGPUCallbackMode_AllowSpontaneous callbacks, which may happen at any time (during a `webgpu.h` function call or on an arbitrary thread).
    - Therefore, calls to the API should never be made in @ref WGPUCallbackMode_AllowSpontaneous callbacks, except where otherwise noted (e.g. @ref wgpuBufferGetMappedRange), due to the possibility of self-deadlock.
      Additionally, applications should take extra care even when accessing their *own* state (and locks) inside the callback.
    - This includes @ref WGPUDeviceDescriptor::uncapturedErrorCallbackInfo, which is always allowed to fire spontaneously.
- Two threads are doing things in parallel, so calls to the API are made while callbacks (or API functions in general) are running on another thread.
    - In general, most API calls are thread-safe (see @ref ThreadSafety).
