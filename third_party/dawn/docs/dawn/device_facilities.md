# Devices

In Dawn the `Device` is a "god object" that contains a lot of facilities useful for the whole object graph that descends from it.
There a number of facilities common to all backends that live in the frontend and backend-specific facilities.
Example of frontend facilities are the management of content-less object caches, or the toggle management.
Example of backend facilities are GPU memory allocators or the backing API function pointer table.

## Frontend facilities

### Error Handling

Dawn (dawn_native) uses the [Error.h](../../src/dawn/native/Error.h) error handling to robustly handle errors.
With `DAWN_TRY` errors bubble up all the way to, and are "consumed" by the entry-point that was called by the application.
Error consumption uses `Device::ConsumeError` that expose them via the WebGPU "error scopes" and can also influence the device lifecycle by notifying of a device loss, or triggering a device loss..

See [Error.h](../../src/dawn/native/Error.h) for more information about using errors.

### Device Lifecycle

The device lifecycle is a bit more complicated than other objects in Dawn for multiple reasons:

 - The device initialization creates facilities in both the backend and the frontend, which can fail.
   When a device fails to initialize, it should still be possible to destroy it without crashing.
 - Execution of commands on the GPU must be finished before the device can be destroyed (because there's noone to "DeleteWhenUnused" the device).
 - On creation a device might want to run some GPU commands (like initializing zero-buffers), which must be completed before it is destroyed.
 - A device can become "disconnected" when a TDR or hot-unplug happens.
   In this case, destruction of the device doesn't need to wait on GPU commands to finish because they just disappeared.

There is a state machine `State` defined in [Device.h](../../src/dawn/native/Device.h) that controls all of the above.
The most common state is `Alive` when there are potentially GPU commands executing.

Initialization of a device looks like the following:

 - `DeviceBase::DeviceBase` is called and does mostly nothing except setting `State` to `BeingCreated` (and initial toggles).
 - `backend::Device::Initialize` creates things like the underlying device and other stuff that doesn't run GPU commands.
 - It then calls `DeviceBase::Initialize` that enables the `DeviceBase` facilities and sets the `State` to `Alive`.
 - Optionally, `backend::Device::Initialize` can now enqueue GPU commands for its initialization.
 - The device is ready to be used by the application!

While it is `Alive` the device can notify it has been disconnected by the backend, in which case it jumps directly to the `Disconnected` state.
Internal errors, or a call to `LoseForTesting` can also disconnect the device, but in the underlying API commands are still running, so the frontend will finish all commands (with `WaitForIdleForDesctruction`) and prevent any new commands to be enqueued (by setting state to `BeingDisconnected`).
After this the device is set in the `Disconnected` state.
If an `Alive` device is destroyed, then a similar flow to `LoseForTesting happens`.

All this ensures that during destruction or forceful disconnect of the device, it properly gets to the `Disconnected` state with no commands executing on the GPU.
After disconnecting, frontend will call `backend::Device::DestroyImpl` so that it can properly free driver objects.

### Toggles

Toggles are booleans that control code paths inside of Dawn, like lazy-clearing resources or using D3D12 render passes.
They aren't just booleans close to the code path they control, because embedders of Dawn like Chromium want to be able to surface what toggles are used by a device (like in about:gpu).

Toogles are to be used for any optional code path in Dawn, including:

 - Workarounds for driver bugs.
 - Disabling select parts of the validation or robustness.
 - Enabling limitations that help with testing.
 - Using more advanced or optional backend API features.

Toggles can be queried using `DeviceBase::IsToggleEnabled`:
```
bool useRenderPass = device->IsToggleEnabled(Toggle::UseD3D12RenderPass);
```

Toggles are defined in a table in [Toggles.cpp](../../src/dawn/native/Toggles.cpp) that also includes their name and description.
The name can be used to require enabling of a toggle or, at the contrary, require the disabling of a toogle.
This is particularly useful in tests so that the two sides of a code path can be tested (for example using D3D12 render passes and not).

Here's an example of a test that is run in the D3D12 backend both with the D3D12 render passes required to be disabled, and in the default configuration.
```
DAWN_INSTANTIATE_TEST(RenderPassTest,
                      D3D12Backend(),
                      D3D12Backend({}, {"use_d3d12_render_pass"}));
// The {} is the list of required enabled toggles, {"..."} the required disabled ones.
```

The toggles state of a device is decided by the adapter when creating it. The steps of device toggles state decision looks as follows:

 - The device toggles state initialized to required device toggles from the DawnTogglesDescriptor chained in device descriptor.
 - The frontend (i.e. not backend-specific) default toggles are set (unless already required) using `TogglesState::Default`.
 - Any backend device toggle that not supported is forced set to a proper state in `Adapter::SetupBackendDeviceToggles` using `TogglesState::ForceSet`.
 - The backend device default toggles are applied (unless already set) in `Adapter::SetupBackendDeviceToggles` using `TogglesState::Default`.

Forcing toggles should only be done when there is no "safe" option for the toggle.
This is to avoid crashes during testing when the tests try to use both sides of a toggle.
For toggles that are safe to enable, like workarounds, the tests can run against the base configuration and with the toggle enabled.
For toggles that are safe to disable, like using more advanced backing API features, the tests can run against the base configuation and with the toggle disabled.

### Immutable object caches

A number of WebGPU objects are immutable once created, and can be expensive to create, like pipelines.
`DeviceBase` contains caches for these objects so that they are free to create the second time.
This is also useful to be able to compare objects by pointers like `BindGroupLayouts` since two BGLs would be equal iff they are the same object.

### Format Tables

The frontend has a `Format` structure that represent all the information that are known about a particular WebGPU format for this Device based on the enabled features.
Formats are precomputed at device initialization and can be queried from a WebGPU format either assuming the format is a valid enum, or in a safe manner that doesn't do this assumption.
A reference to these formats can be stored persistently as they have the same lifetime as the `Device`.

Formats also have an "index" so that backends can create parallel tables for internal informations about formats, like what they translate to in the backing API.

### Object factory

Like WebGPU's device object, `DeviceBase` is an factory with methods to create all kinds of other WebGPU objects.
WebGPU has some objects that aren't created from the device, like the texture view, but in Dawn these creations also go through `DeviceBase` so that there is a single factory for each backend.
