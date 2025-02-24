# MSAA Render To Single Sampled/Multisampled Render To Single Sampled

The `msaa-render-to-single-sampled` feature allows a render pass to include single-sampled attachments while rendering is done with a specified number of samples. When this feature is used, the client doesn't need to explicitly allocate any multi-sammpled color textures. We denote this kind of render passes as "implicit multi-sampled" render passes.

Additional functionalities:
 - Adds `wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled` as chained struct for `wgpu::RenderPassColorAtttachment` to specify number of samples to be rendered for the respective single-sampled attachment.
 - Adds `wgpu::DawnMultisampleStateRenderToSingleSampled` as chained struct for `wgpu::RenderPipelineDescriptor::MultisampleState` to indicate that the render pipeline is going to be used in a "implicit multi-sampled" render pass.

Example Usage:
```
// Create texture with TextureBinding usage.
wgpu::TextureDescriptor desc = ...;
desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

auto texture = device.CreateTexture(&desc);

// Create a render pipeline
wgpu::RenderPipelineDescriptor pipelineDesc = ...;
auto pipeline = device.CreateRenderPipeline(&pipelineDesc);

// Create a render pass with "implicit multi-sampled" enabled.
wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled colorAttachmentRenderToSingleSampledDesc;
colorAttachmentRenderToSingleSampledDesc.implicitSampleCount = 4;

wgpu::RenderPassDescriptor renderPassDesc = ...;
renderPassDesc.colorAttachments[0].view = texture.CreateView();
renderPassDesc.colorAttachments[0].nextInChain
  = &colorAttachmentRenderToSingleSampledDesc;

auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDesc);

renderPassEncoder.SetPipeline(pipeline);
renderPassEncoder.Draw(3);
renderPassEncoder.End();

```

Notes:
 - If a texture needs to be used as an attachment in a "implicit multi-sampled" render pass, it must have `wgpu::TextureUsage::TextureBinding` usage.
 - If a texture is attached to a "implicit multi-sampled" render pass. It must be single-sampled. It mustn't be assigned to the `resolveTarget` field of the the render pass' color attachment.
 - Depth stencil textures can be attached to a "implicit multi-sampled" render pass. But its sample count must match the number specified in one color attachment's `wgpu::DawnRenderPassColorAttachmentRenderToSingleSampled`'s `implicitSampleCount` field.
 - Currently only one color attachment is supported, this could be changed in future.
 - The texture is not supported if it is not resolvable by WebGPU standard. This means this feature currently doesn't work with integer textures.
