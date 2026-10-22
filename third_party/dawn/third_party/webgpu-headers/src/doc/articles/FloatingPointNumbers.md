# Floating-Point Numbers {#FloatingPointNumbers}

## Double-as-Supertype {#DoubleAsSupertype}

Like in the JS API, `double` (aka JavaScript's native `Number` type) is used in several places as a supertype of various numeric values.
Such a value will be numerically downcast to the appropriate subtype depending how it is used,
as specified in the JS API spec.

- In @ref WGPUColor, as a supertype of `f32`/`u32`/`i32`
    - In @ref WGPURenderPassColorAttachment::clearValue, the type depends on the texture format.
    - In @ref wgpuRenderPassEncoderSetBlendConstant, the type is `f32`.
- In @ref WGPUConstantEntry::value, as a supertype of all of the overrideable WGSL types
  (`bool`/`f32`/`u32`/`i32`/`f16` and possibly more).
    - The type depends on the WGSL type of the constant being overridden.

## Nullable Floating-Point Type {#NullableFloatingPointType}

Floating-point-typed (`float`/`double`) values which are nullable or optional use `NaN` to
represent the null value. A value `value` represents the null value iff `isnan(value) != 0`.
(Do not use an equality check with a `NaN` constant, because `NaN == NaN` is false.)

Infinities are invalid. See @ref NonFiniteFloatValueError.

## Non-Finite Float Value Errors {#NonFiniteFloatValueError}

The JavaScript API does not allow non-finite floating-point values (it throws a `TypeError`).

In `webgpu.h`, a value is finite iff `isfinite(value) != 0`.
Using a non-finite value (aside from `NaN` for a @ref NullableFloatingPointType)
results in a validation @ref DeviceError, except on Wasm-on-JS targets where a `TypeError`
thrown by JS **may** be passed unhandled, interrupting Wasm execution.
