# Shader Module Compilation Options

Shader module compilation options may be specified to override Dawn's default compilation behavior.

`wgpu::ShaderModuleCompilationOptions` may be chained on `wgpu::ShaderModuleDescriptor`. If it is,
Dawn will use these compilation options instead of its defaults.

### `wgpu::ShaderModuleCompilationOptions::strictMath`
Enables or disables strict math. When strict math is disabled, generally the compiler will:
- Assume no NaNs
- Assume no Inf
- Assume no signed 0
- Use multiplication by reciprocal instead of division
- Allow algebraic transformations according to associative and distribute properties.

It is implemented only on Metal and D3D.
