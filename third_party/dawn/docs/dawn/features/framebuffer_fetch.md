# Framebuffer Fetch (experimental)

This extension enables support for the [`chromium_experimental_framebuffer_fetch`](../../tint/extensions/chromium_experimental_framebuffer_fetch.md) WGSL extension.

The extension is experimental and might change for example to gain new validation rules (with extension struct) in the future.

It is available on tiler Metal GPUs.

## Validation

In `Device::CreateRenderPipeline` or `Device::CreateRenderPipelineAsync`:
 - For each `@color(N) in : T` fragment in:
   - color target N must exist
   - color target N's format must match T in both component count and base type
