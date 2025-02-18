# YCbCr Vulkan Samplers

Video playback in Vulkan on Android works via so called YCbCr sampling (note:
this is not the only context in which YCbCr sampling can be used on Android, but
for video playback it *must* be used). In a nutshell:

* The AHardwareBuffer containing the video frame has YCbCr information that can
  be queried. This YCbCr information has an opaque "external format" integer
  that must be supplied when creating a sampler to sample this
  AHardwareBuffer. It's possible for this format or other parts of the YCbCr
  info to change from frame to frame (i.e., between AHBs), and hence the
  information must be queried every frame.
* The client must query that information and use it to create an immutable
  sampler
* The client must then package that sampler together with the information of
  the sampled image into a combined image sampler that is inserted into the
  VkDescriptorSetLayout
* The client must insert the sampled image (i.e., an image wrapping the AHB)
  into the VkDescriptorSet
* The client's SPIR-V must either directly create a SampledImage from the
  combined image sampler or must create sampler and texture variables that both
  point to the combined image sampler entry and then create a SampledImage from
  the sampler and texture variables

# Supporting YCbCr Sampling in Dawn: Semantics

A complicating factor of the challenge here is that combined image samplers are
intentionally not a part of the WebGPU specification. Dawn's support for YCbCr
sampling on Android rests on the following features:

* Static samplers, which allow a client to pass a sampler object
  directly in the BindGroupLayout rather than the BindGroup.
* The ability to query YCbCr info from SharedTextureMemory
  and create YCbCr samplers from this info (details below)
* The ability to specify in the BindGroupLayout that a specific static sampler
  entry and texture entry are paired to one another - that sampler will be used
  to sample only that texture, and that texture will be sampled only by that
  sampler. This is done via adding an optional `sampledTextureBinding` to the
  static sampler entry in the BindGroupLayout.

The `y-cb-cr-vulkan-samplers` feature allows specification of VkSamplerYcbcrConversionCreateInfo as
part of the static vulkan sampler descriptor. For this purpose, clients
can supply `YCbCrVkDescriptor` instances when creating samplers and
texture views. Clients can also obtain the YCbCr info for a
SharedTextureMemory instance that was created from an AHardwareBuffer by
querying its properties. Most properties will be created directly from the
corresponding buffer format properties on the underlying AHardwareBuffer. The
two exceptions are as follows:

* `vkChromaFilter`: Will be set to FilterMode::Linear iff
  `VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT` is
  present in the AHB format features and FilterMode::Nearest otherwise
* `forceExplicitReconstruction`: will be set to true iff
  `VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_BIT`
  is present in the AHB format features

# Implementing YCbCr Sampling via Dawn from the Client's Perspective

Putting the above together, a typical client flow would look like this:

* Obtain the AHB for a video frame
* Create a SharedTextureMemory instance from that AHB
* Query that SharedTextureMemory instance for its YCbCr info
* Create a YCbCr sampler using that YCbCr info
* Create a BindGroupLayout that includes that YCbCr sampler with a
  `sampledTextureBinding` annotation that points to the entry of the texture
  that will be sampled (i.e., where the AHB will be passed). That texture entry
  must have format type EXTERNAL. Note: The client might want to cache created
  BGLs for performance; if so, this caching must be keyed off the YCbCr info.
* Create WGSL that references the sampler and texture via their respective
  indices in the BindGroupLayout, just as if they were an "ordinary" sampler and
  texture.
* Create a BindGroup that passes a texture obtained from the SharedTextureMemory
  wrapping the AHB in the texture entry that had been reserved for that texture
  in the BindGroupLayout.

# Implementation Details

* Static samplers are translated to immutable samplers in the Vulkan backend.
* If the static sampler has a `sampledTextureBinding` annotation, it is
  translated to a combined image sampler rather than a regular sampler entry.
  The texture information for that combined image sampler is that which the
  client passed in the corresponding texture entry.
* In BindGroup->VkDescriptorSet translation, the texture is passed at the index
  for the combined image sampler in the VkDescriptorSetLayout.
* In WGSL->SPIR-V translation, any variables created from the texture entry in
  the BindGroup are translated to point to the combined image sampler in the
  VkDescriptorSet (note that variables created from the sampler entry will
  naturally point to the combined image sampler, since the combined image
  sampler is created at the index for the sampler entry).

# Validation

We validate the following:

* YCbCr samplers must be static samplers
* YCbCr samplers must have a `sampledTextureBinding` annotation
* The `sampledTextureBinding` annotation must point to a valid entry
* No two sampler entries can point at the same texture entry
* Textures sampled by YCbCr samplers must have format type External
