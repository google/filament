# Pixel Local Storage (PLS)

This features adds support for efficiently reading and writing per-pixel data that's local to a render pass, with correctly synchronized accesses.
On "explicit" tiler GPUs this additional data is allocate in tile storage similarly to attachments, but on other GPUs the same functionality can be implemented using "fragment shader interlock" and read-write storage textures.

Here is the [design doc](https://docs.google.com/document/d/1djJwQLJcVGnDXOA7nhsweppim-hhbqglB4JNyfZcRqQ) for this feature.

## Concepts

A couple new concepts are added:

 - A `pixel_local` storage class in WGSL that can be used with `var<pixel_local> pls : u32/MyStruct` which declares the pixel local data in the shader.
   It requires a WGSL `enable`, see [`chromium_experimental_framebuffer_fetch`](../../tint/extensions/chromium_experimental_pixel_local.md).
 - Pixel local storage descriptors for both `wgpu::PipelineLayoutDescritor` and `wgpu::RenderPassDescriptor` that describe the layout of the PLS inside a render pass, and are part of pipeline / render pass compatibility.
   These descriptors contain both:

   - The total size of the PLS allocation for the render pass.
   - A number of storage attachments which are textures which will have their corresponding texel loaded/stored into a specific offset of the PLS at the start/end of the render pass.
     (note that on explicit tiler GPUs this is exactly what happens, but on others the load/stores are done directly as "storage texture" operations)
   - Implicit PLS slots (the ones not bound to a storage attachment) start with value 0 and are discarded at the end of the pass.

 - A `wgpu::TextureUsage::StorageAttachment` that allows a texture to be used as a storage attachment in a render pass.
 - The `wgpu::RenderPassEncoder::PixelLocalStorageBarrier` method for use in the non-coherent PLS extension.

## Coherency

The feature comes in two flavors depending on coherency.
The coherent version automatically synchronizes fragment shader accesses to the PLS so that no data race happens (as if there is a critical section between the first and last use of the PLS in an invocation), and enforcing that fragment shaders happen in API order (two fragments invocations from two triangles of the same draw happen in the order of the triangles in the draw).
The non-coherent version allows racy access to the PLS during the whole render pass, but provides a `PixelLocalStorageBarrier()` that prevents races between fragment invocations before and after the barrier.
In particular, there is no way to prevent racy access to the PLS in the same draw with the non-coherent version of the feature.

## New API surface

```cpp
// Add to wgpu::FeatureName
wgpu::FeatureName::PixelLocalStorageNonCoherent
wgpu::FeatureName::PixelLocalStorageCoherent

// Add to wgpu::TextureUsage
wgpu::TextureUsage::StorageAttachment

// Can be chained to a RenderPassDescriptor
struct wgpu::RenderPassPixelLocalStorage : wgpu::ChainedStruct {
    uint64_t totalPixelLocalStorageSize;
    size_t storageAttachmentCount;
    wgpu::RenderPassStorageAttachment* storageAttachments;
};
struct wgpu::RenderPassStorageAttachment : wgpu::ChainedStruct {
    wgpu::LoadOp loadOp;
    wgpu::StoreOp storeOp;
    wgpu::Color clearValue;
    wgpu::TextureView storage;
    uint64_t offset;
};

// Can be chained to a PipelineLayoutDescriptor
struct wgpu::PipelineLayoutPixelLocalStorage : wgpu::ChainedStruct {
    uint64_t totalPixelLocalStorageSize;
    size_t storageAttachmentCount;
    wgpu::PipelineLayoutStorageAttachment* storageAttachments;
};
struct wgpu::PipelineLayoutStorageAttachment : wgpu::ChainedStruct {
    wgpu::TextureFormat format;
    uint64_t offset;
};

// Used for non-coherent.
wgpu::RenderPassEncoder::PixelLocalStorageBarrier();
```

## Validation Rules

 - Only the `R32Uint`, `R32Sint` and `R32Float` texture formats can specify `wgpu::TextureUsage::StorageAttachment`.
 - `StorageAttachment` textures must be single-sampled.
 - In a `PixelLocalStorage` descriptor:

    - The totalPixelLocalStorageSize must be less than a small constant (should eventually get into the limits) currently set at 16 and must be a multiple of 4.
    - The offset of each storage attachment must fit in the total size (with the additional size of that PLS slot) and be a multiple of 4.
    - The format of the storage attachments must be one of the texture formats that allows `StorageAttachment`.
    - Storage attachment views must be for a single subresources (and used at most once in `BeginRenderPass`).
    - Storage attachment views must have the same size of any other attachment used in the render pass.
    - The load and store operations on storage attachments follow the same rules as for color attachments.
    - Two storage attachments must not collide.

 - The PLS definition between a pipeline and a render pass must match.
 - A render pipeline must us a `pixel_local` storage class that's compatible with its layout's PLS descriptor.

   - The totalPLSSize must match the size of the `pixel_local` block.
   - Each slot used for a storage attachment must have a WGSL type that matches that attachment's texture format.
   - Each implicit slot must use the `u32` WGSL type.
