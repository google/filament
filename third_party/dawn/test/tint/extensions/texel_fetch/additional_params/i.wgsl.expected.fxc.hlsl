SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

struct In {
  @location(0)
  a : vec4f,
  @interpolate(flat) @location(1)
  b : vec4f,
  @color(1)
  fbf : vec4i,
}

@fragment
fn f(tint_symbol : In) {
  g(tint_symbol.a.x, tint_symbol.b.y, tint_symbol.fbf.x);
}

fn g(a : f32, b : f32, c : i32) {
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/additional_params/i.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
