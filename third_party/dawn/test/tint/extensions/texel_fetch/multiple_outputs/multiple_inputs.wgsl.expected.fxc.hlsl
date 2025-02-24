SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

struct Out {
  @location(0)
  x : vec4f,
  @location(2)
  y : vec4f,
  @location(4)
  z : vec4f,
}

@fragment
fn f(@color(1) fbf_1 : vec4f, @color(3) fbf_3 : vec4f) -> Out {
  return Out(fbf_1, vec4f(20), fbf_3);
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/multiple_outputs/multiple_inputs.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
