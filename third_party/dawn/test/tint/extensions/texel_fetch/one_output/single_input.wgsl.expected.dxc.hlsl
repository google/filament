SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(0) fbf : vec4f) -> @location(0) vec4f {
  return fbf;
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/one_output/single_input.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
