# Dawn Load Resolve Texture

The `dawn-load-resolve-texture` feature allows a render pass to expand a resolve attachment's pixels into the MSAA attachment's as part of the loading operation.

Additional functionalities:
 - Adds `wgpu::LoadOp::ExpandResolveTexture` enum value to specify that the MSAA attachment will load the pixels from its corresponding resolve texture. This is cheaper than `wgpu::LoadOp::Load` which will load the existing pixels of the MSAA attachment itself.
 - Adds `wgpu::ColorTargetStateExpandResolveTextureDawn` as chained struct for `wgpu::RenderPipelineDescriptor::FragmentState::ColorTargetState`. It has `enabled` flag to indicate that the render pipeline is going to be used in a render pass with `ExpandResolveTexture` load op in the respective color attachment.

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

auto renderPassEncoder2 = encoder.BeginRenderPass(&renderPassDesc2);
renderPassEncoder2.SetPipeline(pipeline);
renderPassEncoder2.Draw(3);
renderPassEncoder2.End();

```

Notes:
 - If a resolve texture is used in a `wgpu::LoadOp::ExpandResolveTexture` operation, it must have `wgpu::TextureUsage::TextureBinding` usage.
 - The `wgpu::ColorTargetStateExpandResolveTextureDawn` chained struct controls the compatibility between a render pipeline and a render pass:
  - If the chained struct is not included in any `wgpu::RenderPipelineDescriptor::FragmentState::ColorTargetState` then the render pipeline **can only** be used on any render pass not using any `ExpandResolveTexture` load op. Whether the render pass has any resolve target doesn't matter.
  - If the chained struct is included in some color targets but **none** of their `enabled` boolean flags are true, then it's the same as above case.
  - If at least one included chained struct has `enabled` = true, then the compatibility's requirements are stricter:
    - If render pipeline's color target `i` has `wgpu::ColorTargetStateExpandResolveTextureDawn.enabled` = true, then the compatible render pass **must** have `ExpandResolveTexture` load op on attachment `i`.
    - If render pipeline's color target `i` has `wgpu::ColorTargetStateExpandResolveTextureDawn.enabled` = false, then the compatible render pass's attachment `i` must have a resolve target and its load op **must not** be `ExpandResolveTexture`.
    - If render pipeline's color target `i` has no `wgpu::ColorTargetStateExpandResolveTextureDawn` included, then the compatible render pass's attachment `i` **must not** have any resolve target.
 - Currently the `ExpandResolveTexture` LoadOp only works on color attachment, this could be changed in future.
 - The texture is not supported if it is not resolvable by WebGPU standard. This means this feature currently doesn't work with integer textures.
 - Using `ExpandResolveTexture` load op on a multiplanar texture's view is not supported currently.
