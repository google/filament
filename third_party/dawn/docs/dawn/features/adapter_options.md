# Adapter Options (Unstable!)

`wgpu::RequestAdapterOptions` may be passed to `wgpuInstanceRequestAdapter` to request an adapter.

Dawn provides a few chained extension structs on `RequestAdapterOptions` to customize the behavior.
Currently, the `WGPUInstance` doesn't provide a way to query support for these features, so these
features may not be suitable for general use yet. Currently, they are used in Dawn's testing, and
in Chromium-specific integration with Dawn.

`DawnTogglesDescriptor` can also be chained on `RequestAdapterOptions` to provide the required adapter toggles. However, the final adapter toggles state also depends on instance toggles inheritance and adapter toggles validation/default-setting.

`dawn::native::Instance::EnumerateAdapters` is a Dawn native-only API that may be used to synchronously
get a list of adapters according to the RequestAdapterOptions. The members are treated as follows:
 - `RequestAdapterOptions::compatibleSurface` is ignored.
 - `RequestAdapterOptions::featureLevel` all returned adapters must support the features and limits in the requested feature level. Devices created from the adapter will default to the capabilities of this feature level.
 - `RequestAdapterOptions::powerPreference` adapters are sorted according to powerPreference such that
   preferred adapters are at the front of the list. It is a preference - so if
  wgpu::PowerPreference::LowPower is passed, the list may contain only integrated GPUs, fallback adapters, or a mix of everything. Implementations *should* try to avoid returning any discrete GPUs when low power is requested if at least one integrated GPU is available.
 - `RequestAdapterOptions::backendType` filters adapters such that only those on a particular backend are discovered. If `WGPURequestAdapterType_Undefined` is passed, all backends may be discovered.
 - `RequestAdapterOptions::forceFallbackAdapter` all returned adapters must be fallback adapters.

If no options are passed to EnumerateAdapters, then it is as if the default `RequestAdapterOptions` are passed.

### `RequestAdapterOptionsGetGLProc`

When discovering adapters on the GLES backend, Dawn uses the provided `RequestAdapterOptionsGetGLProc::getProc` method to load GL procs. `RequestAdapterOptionsGetGLProc::display` indicates the EGLDisplay on which to create an adapter. If `display` is `EGL_NO_DISPLAY`, the current display will be used. This extension struct does nothing on other backends.

### `RequestAdapterOptionsLUID`

When discovering adapters on D3D11 and D3D12, Dawn only discovers adapters matching the provided `RequestAdapterOptionsLUID::adapterLUID`. This extension struct does nothing on other backends.

### `RequestAdapterOptionsD3D11Device`

When discovering adapter on D3D11, Dawn creates an adapter matching the provided `RequestAdapterOptionsD3D11Device::device`, and `wgpu::Device` created from the adapter will share the same D3D11 device from `RequestAdapterOptionsD3D11Device::device`. This extension struct does nothing on other backends.

### `DawnTogglesDescriptor`

When discovering adapters, Dawn will use chained `DawnTogglesDescriptor` as required adapter
toggles. The final toggles state of each result adapter depends on the inheritance of instance
toggles state, the backend-specific validation and the backend-specific default setting.
