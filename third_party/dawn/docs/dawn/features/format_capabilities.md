# Format Capabilities

`wgpu::FeatureName::DawnFormatCapabilities`

Allows querying Adapter::GetFormatCapabilities. This API doesn't yet provide WebGPU related
information about the texture format, but will in the future.

See: https://github.com/webgpu-native/webgpu-headers/issues/283

`wgpu::FeatureName::DawnDrmFormatCapabilities`

Allows querying Adapter::GetFormatCapabilities with a DawnDrmFormatCapabilities chained struct, which
will be populated with DRM info including format modifiers and memory plane counts.

Requires the `wgpu::FeatureName::DawnFormatCapabilities` flag to be enabled as well.
