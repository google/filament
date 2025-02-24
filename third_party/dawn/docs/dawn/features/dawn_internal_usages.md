# Dawn Internal Usages

The `dawn-internal-usages` feature allows adding additional usage which affects how a texture is allocated, but does not affect normal frontend validation.

Adds `WGPUDawnTextureInternalUsageDescriptor` for specifying additional internal usages to create a texture with.

Example Usage:
```
wgpu::DawnTextureInternalUsageDescriptor internalDesc = {};
internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;

wgpu::TextureDescriptor desc = {};
// set properties of desc.
desc.nextInChain = &internalDesc;

device.CreateTexture(&desc);
```

Adds `WGPUDawnEncoderInternalUsageDescriptor` which may be chained on `WGPUCommandEncoderDescriptor`. Setting `WGPUDawnEncoderInternalUsageDescriptor::useInternalUsages` to `true` means that internal resource usages will be visible during validation. ex.) A texture that has `WGPUTextureUsage_CopySrc` in `WGPUDawnEncoderInternalUsageDescriptor::internalUsage`, but not in `WGPUTextureDescriptor::usage` may be used as the source of a copy command.


Example Usage:
```
wgpu::DawnEncoderInternalUsageDescriptor internalEncoderDesc = { true };
wgpu::CommandEncoderDescriptor encoderDesc = {};
encoderDesc.nextInChain = &internalEncoderDesc;

wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);

// This will be valid
wgpu::ImageCopyTexture src = {};
src.texture = texture;
encoder.CopyTextureToBuffer(&src, ...);
```

One use case for this is so that Chromium can use an internal copyTextureToTexture command to implement copies from a WebGPU texture-backed canvas to other Web platform primitives when the swapchain texture was not explicitly created with CopySrc usage in Javascript.
