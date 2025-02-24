SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(1) fbf_1 : vec4f, @color(3) fbf_3 : vec4f) {
  g(fbf_1.r, fbf_3.g);
}

fn g(a : f32, b : f32) {
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/zero_outputs/multiple_inputs.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
