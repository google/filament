# Chromium Experimental Framebuffer Fetch

The `chromium_experimental_framebuffer_fetch` extension adds support for framebuffer inputs to fragment stages in WGSL.
Framebuffer input, or framebuffer fetch, lets a fragment shader read the value of framebuffer attachments for the current fragment.
It synchronizes the read with all previous writes to the same fragment (in API order).

## Status

Framebuffer Fetch support in Tint is experimental and might change as we gain implementation experience.
It will also be updated to follow an upstream specification if one is made at some point.
Specification work in the WebGPU group hasn't started.

## Pseudo-specification

This adds a new builtin attribute `@color` which takes a `u32` argument and is only allowed on `@fragment` entry point scalar / vector inputs.
Two input variables cannot have the same `@color` attribute.

## Example usage

```
@fragment fn main(@color(0) vec4f previousColor) -> @location(0) vec4f {
    let color = computeThisFragmentColor();
    return myBlend(previousColor, color);
}
```

## To-dos

 - Are f16 types allowed for the `@color` inputs?
 - Are any changes needed for Vulkan support?
