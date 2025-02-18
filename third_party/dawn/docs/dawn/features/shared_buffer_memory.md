# Shared Buffer Memory

## Overview

Shared Buffer Memory refers to a superset of features that allow Dawn to import externally allocated buffers.

- `wgpu::FeatureName::SharedBufferMemoryD3D12Resource`

```c++
wgpu::SharedBufferMemoryFooBarDescriptor fooBarDesc = {
  .fooBar = ...,
};
wgpu::SharedBufferMemoryDescriptor desc = {};
desc.nextInChain = &fooBarDesc;

wgpu::SharedBufferMemory memory = device.CreateSharedBufferMemory(&desc);
```

After creating the memory, the supported `wgpu::BufferUsage` can be queried.
```c++
wgpu::SharedBufferMemoryProperties properties;
memory.GetProperties(&properties);

switch (properties.usage) {
  // ... handle supported usage
}
```

Then, a buffer can be created from it. For example:
```c++
wgpu::BufferDescriptor bufferDesc = {
  // usage must be a subset of the buffer memory's usage
  .usage = properties.usage
};
wgpu::Buffer buffer = memory.CreateBuffer(&bufferDesc);
```

A buffer created from shared buffer memory is not valid to use inside a queue operation until access to the memory is explicitly started using `BeginAccess`. Access is ended using `EndAccess`. For example:

```c++
wgpu::BufferDescriptor bufferDesc = { ... };
wgpu::Buffer buffer = memory.CreateBuffer(&bufferDesc);

// It is valid to create a bind group and encode commands
// referencing the buffer.
wgpu::BindGroup bg = device.CreateBindGroup({..., buffer, ... });
wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
wgpu::ComputePass pass = encoder.BeginComputePass();
// ...
pass.SetBindGroup(0, bg);
// ...
pass.End();
wgpu::CommandBuffer commandBuffer = encoder.Finish();

// Begin/EndAccess must wrap usage of the buffer on the queue.
wgpu::SharedBufferMemoryBeginAccessDescriptor beginAccessDesc = { ... };
memory.BeginAccess(buffer, &beginAccessDesc);

queue.writeBuffer(buffer, ...);
queue.submit(1, &commandBuffer);

wgpu::SharedBufferMemoryEndAccessState endAccessDesc = { ... };
memory.EndAccess(buffer, &endAccessDesc);
```

# Uniform Usage Restriction

Using wgpu:BufferUsage::Uniform with a buffer created from SharedBufferMemory is not allowed due to an alignment restriction on D3D12 when creating a constant buffer view. It is possible this restriction could be removed in the future if additional alignment restrictions are placed on the provided SharedBufferMemory during import.

# Mappable Buffers

A buffer created from shared buffer memory cannot be mapped until access to the memory is explicitly started using `BeginAccess`. The buffer must be unmapped before calling `EndAccess`.

# Concurrent Access

Multiple buffers may be created from the same SharedBufferMemory, but only one buffer may have access to the memory at a time.

# SharedBufferMemory Types

`SharedBufferMemoryDescriptor` accepts different chained structs corresponding to the memory type.

# Synchronization

The Begin/End access descriptors are used to pass [Shared Fences](./shared_fence.md) necessary to synchronize buffer states between Dawn and external code.

When calling `BeginAccess`, Dawn will wait for any passed shared fences to be signaled before the buffer is used on the GPU. This will ensure any GPU work occurring outside of Dawn is finished before Dawn accesses the memory.

When calling `EndAccess`, the shared fences for the buffer's last usages will be exported. This allows code external to Dawn to be notified when Dawn's GPU work on the shared memory is complete.