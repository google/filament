# Transient Attachments

The `transient-attachments` feature allows creation of attachments that allow
render pass operations to stay in tile memory, avoiding VRAM traffic and
potentially avoiding VRAM allocation for the textures.

Example Usage:
```
wgpu::TextureDescriptor desc;
desc.format = wgpu::TextureFormat::RGBA8Unorm;
desc.size = {1, 1, 1};
desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TransientAttachment;

auto transientTexture = device.CreateTexture(&desc);

// Can now create views from the texture to serve as transient attachments, e.g.
// as color attachments in a render pipeline.
```

Notes:
- Only supported usage is wgpu::TextureUsage::RenderAttachment |
wgpu::TextureUsage::TransientAttachment
- It is not possible to load from or store to TextureViews that are used as
transient attachments
