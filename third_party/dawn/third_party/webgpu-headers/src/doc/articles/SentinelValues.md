# Sentinel Values {#SentinelValues}

## Undefined and Null

Since WebGPU is defined first as a JavaScript API, it uses the JavaScript value
`undefined` in many places to indicate the lack of a value.

This is usually used in dictionaries, where, for example, `{}` and
`{ powerPreference: undefined }` are equivalent, and distinct from both
`{ powerPreference: "high-performance" }` and `{ powerPreference: "low-power" }`.

It may also be used in functions/methods. For example, `GPUBuffer`'s
`getMappedRange(optional GPUSize64 offset = 0, optional GPUSize64 size)`
can be called equivalently as `b.getMappedRange()`, `b.getMappedRange(0)`,
`b.getMappedRange(undefined)`, or `b.getMappedRange(undefined, undefined)`.

To represent `undefined` in C, `webgpu.h` uses `NULL` where possible (anything
behind a pointer, including objects), `*_UNDEFINED` sentinel numeric values
(usually `UINT32_MAX` or similar), `*_Undefined` enum values (usually `0`),
or other semantic-specific names (like `WGPU_WHOLE_SIZE`).

The place that uses the type will define what to do with an undefined or
other sentinel value. It may be:

- Required, in which case an error is produced.
- Defaulting, in which case it trivially defaults to some other value.
- Optional, in which case the API accepts the sentinel value and handles it
  (usually this is either a special value or it has more complex defaulting,
  for example depending on other values).

## C-Specific Sentinel Values

Undefined and null values are also used in C-specific ways in place of
WebIDL's more flexible typing:

- \ref WGPUStringView has a special null value
- \ref WGPUFuture has a special null value
- Special cases to indicate the parent struct is null, avoiding extra layers of
  pointers just for nullability:
    - \ref WGPUVertexBufferLayout::stepMode = \ref WGPUVertexStepMode_Undefined with \ref WGPUVertexBufferLayout::attributeCount
    - \ref WGPUBufferBindingLayout::type = \ref WGPUBufferBindingType_BindingNotUsed
    - \ref WGPUSamplerBindingLayout::type = \ref WGPUSamplerBindingType_BindingNotUsed
    - \ref WGPUTextureBindingLayout::sampleType = \ref WGPUTextureSampleType_BindingNotUsed
    - \ref WGPUStorageTextureBindingLayout::access = \ref WGPUStorageTextureAccess_BindingNotUsed
    - \ref WGPURenderPassColorAttachment::view = `NULL`
    - \ref WGPUColorTargetState::format = \ref WGPUTextureFormat_Undefined
- \ref NullableFloatingPointType
