SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(0) fbf : vec4f) {
  g(fbf.y);
}

fn g(a : f32) {
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/zero_outputs/single_input.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
