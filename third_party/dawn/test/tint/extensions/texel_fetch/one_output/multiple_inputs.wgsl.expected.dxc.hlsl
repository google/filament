SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(1) fbf_1 : vec4f, @color(3) fbf_3 : vec4f) -> @location(0) vec4f {
  return (fbf_1 + fbf_3);
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/one_output/multiple_inputs.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
