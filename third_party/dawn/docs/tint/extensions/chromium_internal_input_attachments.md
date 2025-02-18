# Chromium Internal Input Attachments

The `chromium_internal_input_attachments` extension adds support for input attachment global variables to WGSL.
This is similar to Vulkan's `subpassInput` variable.

## Status

Input attachments support in Tint is highly experimental and only meant to be used in internal transforms at this stage.
This extension is only relevant to SPIRV backend atm.
Specification work in the WebGPU group hasn't started.

## Pseudo-specification

This extension adds:
- A new `input_attachment<T>` type in the `handle` address space, where `T` is `u32`, `i32` or `f32`. It's only allowed on module-scoped variable declarations.
  - This will be mapped to `OpTypeImage` with Dim=`SubpassData` in SPIRV.
- A new `input_attachment_index(n)` attribute where `n` is a `u32` or `i32` with a positive value. Unlike `binding` which specifies the binding point in a BindGroup, this attribute specifies the index of the input attachment in the render pass descriptor. This is required for `input_attachment<T>` type.
  - This is equivalent to `InputAttachmentIndex` decoration in SPIRV.
- A new `inputAttachmentLoad` builtin function with signature:
  `inputAttachmentLoad(input_attachment<T>) -> vec4<T>`
  - This will emit `OpImageRead` function call in SPIRV.
- All of the above are only available in fragment state.

## Example usage

```
@group(0) @binding(0) @input_attachment_index(0)
    var input_tex : input_attachment<f32>;

@fragment fn main() -> @location(0) vec4f {
    return inputAttachmentLoad(input_tex);
}
```
