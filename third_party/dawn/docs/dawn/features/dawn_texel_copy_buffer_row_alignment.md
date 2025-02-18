# Dawn Texel Copy Buffer Row Alignment

The `dawn-texel-copy-buffer-row-alignment` feature exposes the alignment restriction of `byetsPerRow` in `wgpu::TexelCopyBufferLayout`. Each backend may have its own min alignment value. Without this feature, the alignment must be 256 for all backends.

Additional functionalities:
 - Adds `wgpu::DawnTexelCopyBufferRowAlignmentLimits` as chained struct for `wgpu::SupportedLimits`. It has a member `minTexelCopyBufferRowAlignment` to indicate the alignment limit of the current device.


Notes:
 - Even with this feature enabled, the alignment clients actually use, still needs to respect 'bytes-per-texel-block', and should be the max of them.
 - The feature currently is only available on D3D11 backend.
