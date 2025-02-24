# Multi Planar Formats

Video is often decoded to formats with a full resolution luminance image and a lower resolution chrominance image.
This is represented in GPU APIs with multi-planar color texture formats where plane 0 is luminance, plane 1 is chrominance and plane 2 an optional alpha channel. It is important to support these formats in Dawn because in combination with `wgpu::ExternalTexture` they can provide 0-copy access to video data with less memory bandwidth than if it were decoded to `RGBA8Unorm` textures.

Dawn has a number of optional features that give access to some of these formats and texture capabilities.
Note however that Dawn doesn't specify the semantics of the planes (luminance vs. chrominance), but instead just exposes multiple planes with varying sizes and formats.

Multi planar formats introduce new `wgpu::TextureAspect` values `Plane0Only`, `Plane1Only` and `Plane2Only` to allow using a single aspect of multi planar textures in `TextureView` creation or copy operations. All multi planar formats support `wgpu::TextureUsage::TextureBinding` and `wgpu::TextureUsage::CopySrc` without any need for another optional feature (and cannot be created directly, only imported with `SharedTextureMemory`).

TODO(dawn:551): It seems these aspects enums are always allowed. Maybe they should be gated on the base feature?

## `DawnMultiPlanarFormats`

This feature adds support for "NV12", a common format for decoded SDR videos.
It is composed of:

 - a full res `R8Unorm` plane 0 (luminance).
 - a 1/4 res `RG8Unorm` plane 1 (chrominance).

For historical reasons the feature is called `DawnMultiPlanarFormats`.
This format is called `wgpu::TextureFormat::R8BG8Biplanar420Unorm`.

TODO(dawn:551): Rename to MultiPlanarFormatNV12 ?

## `MultiPlanarFormatP010`

This feature adds support for "P010", a common format for decoded HDR videos.
It is composed of:

 - a full res `R16Unorm` plane 0 (luminance) with only the first 10 bits used in the decoding of the videos.
 - a 1/4 res `RG16Unorm` plane 1 (chrominance) with only the first 10 bits of each channel used in the decoding of the videos.

This format is called `wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm`.

TODO(dawn:551) is it possible to merge this feature with the nv12 feature?

## `MultiPlanarFormatNV12A` (Experimental!)

This feature adds support for "NV12A", a format for decoded SDR videos with an alpha channel.
It is composed of:

 - a full res `R8Unorm` plane 0 (luminance).
 - a 1/4 res `RG8Unorm` plane 1 (chrominance).
 - a full res `R8Unorm` plane 2 (alpha).

This format is called `wgpu::TextureFormat::R8BG8A8Triplanar420Unorm`.

At the moment it is only available on macOS >= 10.15, iOS >= 13.0.

## `MultiPlanarFormatNV16` (Experimental!)

This feature adds support for "NV16", a common format for 8-bit decoded 4:2:2 videos.
It is composed of:

 - a full res `R8Unorm` plane 0 (luminance).
 - a 1/2 res `RG8Unorm` plane 1 (chrominance).

This format is called `wgpu::TextureFormat::R8BG8Biplanar422Unorm`.

At the moment it is only available on macOS >= 11.0, iOS >= 14.0.

## `MultiPlanarFormatNV24` (Experimental!)

This feature adds support for "NV24", a common format for 8-bit decoded 4:2:2 videos.
It is composed of:

 - a full res `R8Unorm` plane 0 (luminance).
 - a full res `RG8Unorm` plane 1 (chrominance).

This format is called `wgpu::TextureFormat::R8BG8Biplanar444Unorm`.

At the moment it is only available on macOS >= 11.0, iOS >= 14.0.

## `MultiPlanarFormatP210` (Experimental!)

This feature adds support for "P210", a common format for 10-bit decoded 4:2:2 videos.
It is composed of:

 - a full res `R16Unorm` plane 0 (luminance) with only the first 10 bits used in the decoding of the videos.
 - a 1/2 res `RG16Unorm` plane 1 (chrominance) with only the first 10 bits of each channel used in the decoding of the videos.

This format is called `wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm`.

At the moment it is only available on macOS >= 10.13, iOS >= 11.0.

## `MultiPlanarFormatP410` (Experimental!)

This feature adds support for "P410", a common format for 10-bit decoded 4:4:4 videos.
It is composed of:

 - a full res `R16Unorm` plane 0 (luminance) with only the first 10 bits used in the decoding of the videos.
 - a full res `RG16Unorm` plane 1 (chrominance) with only the first 10 bits of each channel used in the decoding of the videos.

This format is called `wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm`.

At the moment it is only available on macOS >= 10.13, iOS >= 11.0.

## `MultiPlanarFormatExtendedUsages`

Adds the ability to create multi-planar textures with `device.CreateTexture()` and the `wgpu::TextureUsage::CopyDst`usage.

## `MultiPlanarRenderTargets`

Adds the `wgpu::TextureUsage::RenderAttachment` usage to multi planar formats.
