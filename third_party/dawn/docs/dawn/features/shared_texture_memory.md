# Shared Texture Memory

## Overview

A variety of features may be used to import externally allocated shared texture memory.

- `wgpu::FeatureName::SharedTextureMemoryAHardwareBuffer`
- `wgpu::FeatureName::SharedTextureMemoryDmaBuf`
- `wgpu::FeatureName::SharedTextureMemoryOpaqueFD`
- `wgpu::FeatureName::SharedTextureMemoryZirconHandle`
- `wgpu::FeatureName::SharedTextureMemoryDXGISharedHandle`
- `wgpu::FeatureName::SharedTextureMemoryD3D11Texture2D`
- `wgpu::FeatureName::SharedTextureMemoryIOSurface`

Each memory type uses an extension struct chained on `wgpu::SharedTextureMemoryDescriptor` to create the memory. For example:

```c++
wgpu::SharedTextureMemoryFooBarDescriptor fooBarDesc = {
  .fooBar = ...,
};
wgpu::SharedTextureMemoryDescriptor desc = {};
desc.nextInChain = &fooBarDesc;

wgpu::SharedTextureMemory memory = device.CreateSharedTextureMemory(&desc);
```

After creating the memory, the properties such as the supported `wgpu::TextureUsage` can be queried.
```c++
wgpu::SharedTextureMemoryProperties properties;
memory.GetProperties(&properties);

switch (properties.usage) {
  // ... handle supported usage
}
```

Then, a texture can be created from it. For example:
```c++
wgpu::TextureDescriptor textureDesc = {
  // usage must be a subset of the texture memory's usage
  .usage = properties.usage
};
wgpu::Texture texture = memory.CreateTexture(&textureDesc);
```

Note: There are restrictions on the configuration that can be requested for
textures created from SharedTextureMemory objects:
- they must be single-sampled
- they must be 2D
- they must have a single mip level
- they must have an array layer count of 1

Textures created from shared texture memory are not valid to use inside a queue operation until access to the memory is explicitly started using `BeginAccess`. Access is ended using `EndAccess`. For example:

```c++
wgpu::TextureDescriptor textureDesc = { ... };
wgpu::Texture texture = memory.CreateTexture(&textureDesc);

// It is valid to create a bind group and encode commands
// referencing the texture.
wgpu::BindGroup bg = device.CreateBindGroup({..., texture, ... });
wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
wgpu::ComputePass pass = encoder.BeginComputePass();
// ...
pass.SetBindGroup(0, bg);
// ...
pass.End();
wgpu::CommandBuffer commandBuffer = encoder.Finish();

// Begin/EndAccess must wrap usage of the texture on the queue.
wgpu::SharedTextureMemoryBeginAccessDescriptor beginAccessDesc = { ... };
memory.BeginAccess(texture, &beginAccessDesc);

queue.writeTexture(texture, ...);
queue.submit(1, &commandBuffer);

wgpu::SharedTextureMemoryEndAccessState endAccessDesc = { ... };
memory.EndAccess(texture, &endAccessDesc);
```

Multiple textures may be created from the same SharedTextureMemory, but only one texture may have access to the memory at a time.

The Begin/End access descriptors are used to pass data necessary to update and synchronize texture state since the texture may have been externally used. Chained structs that are supported or required vary depending on the memory type. These access descriptors communicate the texture's initialized state. They are also used to pass [Shared Fences](./shared_fence.md) to synchronize access to the texture. When passed in BeginAccess, the texture will wait for the shared fences to pass before use on the GPU, and on EndAccess, the shared fences for the texture's last usage will be exported. Passing shared fences to BeginAccess allows Dawn to wait on externally-enqueued GPU work, and exporting shared fences from EndAccess then allows other external code to wait for Dawn's GPU work to complete.

TODO(crbug.com/dawn/1745): additional documentation

Work-in-progress: https://docs.google.com/document/d/1uRGL6vE1mSbpWd2v_KU5--RT5EjTXtruwiC7Ri3ZKz4/edit
