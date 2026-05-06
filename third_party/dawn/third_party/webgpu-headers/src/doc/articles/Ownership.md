# Ownership {#Ownership}

Objects in `webgpu.h` are refcounted via the `AddRef` and `Release` functions.
The refcount only changes when these methods are called explicitly (not, for example, in \ref wgpuCommandEncoderFinish or \ref wgpuQueueSubmit).

Applications are *not* required to maintain refs to WebGPU objects which are internally used by other WebGPU objects (CommandBuffer→BindGroup, BindGroup→TextureView, TextureView→Texture, etc.); `webgpu.h` implementations must maintain internal references, as needed, to be internally memory safe. These *internal* references do *not* make it safe to use objects that have ever reached an *external* refcount of 0.

Memory for variable-sized outputs from the API (message strings, capability arrays, etc.) is managed in different ways depending on whether they are returned values or callback arguments.

## Returned with Ownership {#ReturnedWithOwnership}

Objects returned directly from the API (e.g. \ref WGPUBuffer from \ref wgpuDeviceCreateBuffer and \ref WGPUTexture via \ref WGPUSurfaceTexture from \ref wgpuSurfaceGetCurrentTexture) start with one application-owned ref.
The application must `Release` this ref before losing the pointer.
(The returned object may _also_ have other refs, either API-owned refs or existing application-owned refs, but this should not be relied upon.)

Variable-sized outputs returned from the API (e.g. the strings in \ref WGPUAdapterInfo, from \ref wgpuAdapterGetInfo) are application-owned.
The application must call the appropriate `FreeMembers` function (e.g. \ref wgpuAdapterInfoFreeMembers) before losing the member pointers.
`FreeMembers` functions do not traverse the [struct chain](@ref StructChaining) and must be called separately on each struct (that has a `FreeMembers` function) in the chain.

Note that such functions will *not* free any previously-allocated data: overwriting an output structure without first releasing ownership will leak the allocations; e.g.:

- Overwriting the strings in \ref WGPUAdapterInfo with \ref wgpuAdapterGetInfo without first calling \ref wgpuAdapterInfoFreeMembers.
- Overwriting the `texture` in \ref WGPUSurfaceTexture with \ref wgpuSurfaceGetCurrentTexture without first calling \ref wgpuTextureRelease.

Note also that some structs with `FreeMembers` functions may be used as both inputs and outputs. In this case `FreeMembers` must only be used if the member allocations were made by the `webgpu.h` implementation (as an output), not if they were made by the application (as an input).

## Releasing a Device object {#DeviceRelease}

Unlike other destroyable objects, releasing the last (external) ref to a device causes it to be automatically destroyed, if it isn't already lost or destroyed. Though the device object is no longer valid to use after releasing the last ref, the direct effects of @ref wgpuDeviceDestroy() still take effect:

- It destroys all buffers on the device, which will unmap them and abort any pending map requests.
- It loses the device and triggers the DeviceLost event.

Because the device's last ref has already been released, DeviceLost callbacks triggered in this way will *not* receive a pointer to the device object. More specifically, freeing the last ref:

1. Sets a flag on the DeviceLost future (if it is uncompleted, even if it is already ready) indicating it should pass a null @ref WGPUDevice to the callback.
1. Calls @ref wgpuDeviceDestroy(), readying the DeviceLost future.
    - This may call the DeviceLost callback, if it is @ref WGPUCallbackMode_AllowSpontaneous.
1. Decrements the refcount, bringing it to 0.

## Callback Arguments {#CallbackArgs}

Output arguments passed from the API to application callbacks include objects and message strings, which are passed to most callbacks.

### Passed with Ownership {#PassedWithOwnership}

Usually, object arguments passed to callbacks start with one application-owned ref, which the application must free before losing the pointer.

### Passed without Ownership {#PassedWithoutOwnership}

A.k.a. "pass by reference".

Sometimes, object arguments passed to callbacks are non-owning (such as the \ref WGPUDevice in \ref WGPUDeviceLostCallback) - the application doesn't need to free them.

Variable-sized and [struct-chained](@ref StructChaining) outputs passed from the API to callbacks (such as message strings in most callbacks) are always owned by the API and passed without ownership. They are guaranteed to be valid only during the callback's execution, after which the pointers passed to the callback are no longer valid.

### Implementation-Allocated Struct Chain {#ImplementationAllocatedStructChain}

Some callback arguments contain [chained structs](@ref StructChaining).
These are allocated by the implementation. They:

- May be chained in any order.
- May contain chain members not known to the application (e.g. implementation-specific extensions or extensions added in a later version of `webgpu.h`).

Applications must handle these cases gracefully, by traversing the struct chain and ignoring any structs in the chain that have @ref WGPUChainedStruct::sType values other than the ones they're interested in.

Implementations may consider injecting bogus structs into such chains, e.g. `{.sType=0xDEADBEEF}` in debug builds, to help developers catch invalid assumptions in their applications.
