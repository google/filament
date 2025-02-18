SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@location(0) a : vec4f, @interpolate(flat) @location(1) b : vec4f, @color(0) fbf : vec4f) {
  g(a.x, b.y, fbf.x);
}

fn g(a : f32, b : f32, c : f32) {
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/additional_params/j.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
