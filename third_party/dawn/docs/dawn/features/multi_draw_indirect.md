# Multi Draw Indirect

The multi-draw-indirect feature allows issuing multiple draws with a single command. This is useful for rendering thousands if not millions of objects in GPU driven rendering.

It introduces the following new commands:

- `wgpu::CommandEncoder::multiDrawIndirect(wgpu::Buffer indirectBuffer, uint64_t indirectOffset, uint32_t maxDrawCount, wgpu::Buffer countBuffer, uint64_t countOffset)`
- `wgpu::CommandEncoder::multiDrawIndexedIndirect(wgpu::Buffer indirectBuffer, uint64_t indirectOffset, uint32_t maxDrawCount, wgpu::Buffer countBuffer, uint64_t countOffset)`

This feature aims to overcome the shortcomings of `drawIndirect` and `drawIndexedIndirect` commands, which can only issue a single draw call at a time from a certain region of a buffer.
In GPU driven rendering, the compute passes can generate the thousands of draw calls and specify how many draw calls to issue, which can be stored in a buffer and then executed.
With `drawIndirect` and `drawIndexedIndirect`, the CPU has to issue every single draw call and if the number of draw calls is not known ahead of time, the CPU has to read back the number of draw calls from the GPU, which can be a performance bottleneck.
The alternative is to issue maxDrawCount commands and zero out the draw calls that are not needed, but that wastes CPU and GPU resources.
The bottleneck can be avoided by using this feature.


## Usage

`wgpu::FeatureName::MultiDrawIndirect` feature must be enabled to use the commands. Most desktop GPUs support this feature.

```cpp
// Create a buffer to store the draw calls, and the number of draw calls which is a uint32_t.
// Number of draw calls can be stored in another buffer with indirect usage.
wgpu::BufferDescriptor drawCallsBufferDesc = {
	.size = sizeof(DrawCall) * 100 + sizeof(uint32_t),
	.usage = wgpu::BufferUsage::Indirect | wgpu::BufferUsage::Storage,
};
wgpu::Buffer drawCallsBuffer = device.CreateBuffer(&drawCallsBufferDesc);

// Fill the buffer with draw calls and the uint32_t specifying the number of draw calls ...
wgpu::Buffer vertexBuffer = ...;
wgpu::BindGroup bindGroup = ...;
wgpu::CommandEncoder encoder = ...;
wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(...);

// In the render pass, issue the draw calls
{
	...
	renderPassEncoder.SetBindGroup(...);
	renderPassEncoder.SetPipeline(...);
	renderPassEncoder.SetVertexBuffer(0, vertexBuffer);
	renderPassEncoder.MultiDrawIndirect(drawCallsBuffer, 0, 32, drawCallsBuffer, 1024);
	...
}

renderPassEncoder.EndPass();
```

## Restrictions

- The number of draw calls executed is `min(maxDrawCount, countBuffer)`, where countBuffer is the number of draw calls specified in the buffer with the offset.
- The indirect buffer must be created with `wgpu::BufferUsage::Indirect` usage and be large enough to store `maxDrawCount` draw calls.