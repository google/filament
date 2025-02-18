# Chromium Experimental Push Constant

The `chromium_experimental_push_constant` extension adds support for push constant global variables to WGSL.
Push constants are small amounts of data that are passed to the shader and are expected to be more lightweight to set / modify than uniform buffer bindings.
The concept of push constant comes from Vulkan but D3D12 has similar "root constants".
Metal doesn't have the same concept but push constants can be efficiently implemented with the `setBytes` family of command encoder methods.

## Status

Push constant support in Tint is highly experimental and only meant to be used in internal transforms at this stage.
Specification work in the WebGPU group hasn't started.

## Pseudo-specification

This extension adds a new `push_constant` address space that's only allowed on global variable declarations.
Push constant variables must only contain 32bit data types (or aggregates of such types).
Push constant variable declarations must not have an initializer.
It is an error for a entry point to statically use more than one `push_constant` variable.

## Example usage

```
var<push_constant> draw_id : u32;

@fragment fn main() -> @location(0) u32 {
    return draw_id;
}
```
