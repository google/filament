writeTexture + copyBufferToTexture + copyTextureToBuffer validation tests.

Test coverage:
* resource usages:
  - texture_usage_must_be_valid: for GPUTextureUsage::COPY_SRC, GPUTextureUsage::COPY_DST flags.
  - buffer_usage_must_be_valid: for GPUBufferUsage::COPY_SRC, GPUBufferUsage::COPY_DST flags.

* textureCopyView:
  - texture_must_be_valid: for valid, destroyed, error textures.
  - sample_count_must_be_1: for sample count 1 and 4.
  - mip_level_must_be_in_range: for various combinations of mipLevel and mipLevelCount.
  - format: for all formats with full and non-full copies on width, height, and depth.
  - texel_block_alignment_on_origin: for all formats and coordinates.

* bufferCopyView:
  - buffer_must_be_valid: for valid, destroyed, error buffers.
  - bytes_per_row_alignment: for bytesPerRow to be 256-byte aligned or not, and bytesPerRow is required or not.

* linear texture data:
  - bound_on_rows_per_image: for various combinations of copyDepth (1, >1), copyHeight, rowsPerImage.
  - offset_plus_required_bytes_in_copy_overflow
  - required_bytes_in_copy: testing minimal data size and data size too small for various combinations of bytesPerRow, rowsPerImage, copyExtent and offset. for the copy method, bytesPerRow is computed as bytesInACompleteRow aligned to be a multiple of 256 + bytesPerRowPadding * 256.
  - texel_block_alignment_on_rows_per_image: for all formats.
  - offset_alignment: for all formats.
  - bound_on_offset: for various combinations of offset and dataSize.

* texture copy range:
  - 1d_texture: copyExtent.height isn't 1, copyExtent.depthOrArrayLayers isn't 1.
  - texel_block_alignment_on_size: for all formats and coordinates.
  - texture_range_conditons: for all coordinate and various combinations of origin, copyExtent, textureSize and mipLevel.

TODO: more test coverage for 1D and 3D textures.
