# Dawn partial Load Resolve Texture

The `dawn-partial-load-resolve-texture` feature is an extension to `dawn-load-resolve-texture`, which in addition allows to specify a rect sub-region of texture, where load and resolve will take effect only. The feature can't be available unless `dawn-load-resolve-texture` is available.

Additional functionalities:
 - Adds `wgpu::RenderPassDescriptorExpandResolveRect` as chained struct for `wgpu::RenderPassDescriptor`. It defines a rect of {`x`, `y`, `width`, `height`} to indicate that expanding and resolving are only performed partially on the texels within this rect region of texture. The texels outside of the rect are not impacted.

Example Usage:
```
// Create MSAA texture
wgpu::TextureDescriptor desc = ...;
desc.usage = wgpu::TextureUsage::RenderAttachment;
desc.sampleCount = 4;

auto msaaTexture = device.CreateTexture(&desc);


// Create resolve texture with TextureBinding usage.
wgpu::TextureDescriptor desc = ...;
desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

auto resolveTexture = device.CreateTexture(&desc);

// Create a render pass which will discard the MSAA texture.
wgpu::RenderPassDescriptor renderPassDesc = ...;
renderPassDesc.colorAttachments[0].view = msaaTexture.CreateView();
renderPassDesc.colorAttachments[0].resolveTarget
 = resolveTexture.CreateView();
renderPassDesc.colorAttachments[0].storeOp
 = wgpu::StoreOp::Discard;

auto renderPassEncoder = encoder.BeginRenderPass(&renderPassDesc);
renderPassEncoder.Draw(3);
renderPassEncoder.End();

// Create a render pipeline with wgpu::ColorTargetStateExpandResolveTextureDawn.
wgpu::ColorTargetStateExpandResolveTextureDawn pipelineExpandResolveTextureState;
pipelineExpandResolveTextureState.enabled = true;

wgpu::RenderPipelineDescriptor pipelineDesc = ...;
pipelineDesc.multisample.count = 4;

pipelineDesc->fragment->targets[0].nextInChain = &pipelineExpandResolveTextureState;

auto pipeline = device.CreateRenderPipeline(&pipelineDesc);

// Create another render pass with "ExpandResolveTexture" LoadOp.
// Even though we discard the previous content of the MSAA texture,
// the old pixels of the resolve texture will be reserved across
// render passes.
wgpu::RenderPassDescriptor renderPassDesc2 = ...;
renderPassDesc2.colorAttachments[0].view = msaaTexture.CreateView();
renderPassDesc2.colorAttachments[0].resolveTarget
 = resolveTexture.CreateView();
renderPassDesc2.colorAttachments[0].loadOp
 = wgpu::LoadOp::ExpandResolveTexture;

// If there is no need to expand and resolve the whole texture,
// RenderPassDescriptorExpandResolveRect can be used to specify a
// subregion of texture to be updated only.
wgpu::RenderPassDescriptorExpandResolveRect rect{};
rect.x = rect.y = 0;
rect.width = rect.height = 32;
renderPassDesc2.nextInChain = &rect;

auto renderPassEncoder2 = encoder.BeginRenderPass(&renderPassDesc2);
renderPassEncoder2.SetPipeline(pipeline);
renderPassEncoder2.Draw(3);
renderPassEncoder2.End();

```

Notes:
 - In case that the target size of a render pass is very large, the cost of using `wgpu::LoadOp::ExpandResolveTexture` can be rather expensive, as it always assumes full-size expand and resolve. More commonly in reality, each frame we only need to re-draw a small damage region, of which UI frameworks usually have the knowledge, instead of the full window, or webpage. This feature aims to eliminate the waste by doing partial expand and resolve with the hint of `wgpu::RenderPassDescriptorExpandResolveRect`, the actual damage region.
 - The feature currently is only available on dawn d3d11 backend. Internally, both expand and resolve are implemented with a dedicated `wgpu::RenderPipeline`. `wgpu::RenderPassEncoder::APISetScissorRect` is used to set the scissor rect to `wgpu::RenderPassDescriptorExpandResolveRect`, when using the pipeline. The major difference is that expand lives in the original render pass, while resolve requires a separate one.