SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

fn g(a : f32, b : f32, c : f32) {
}

struct In {
  @builtin(position)
  pos : vec4f,
  @location(0)
  uv : vec4f,
  @color(0)
  fbf : vec4f,
}

@fragment
fn f(tint_symbol : In) {
  g(tint_symbol.pos.x, tint_symbol.uv.x, tint_symbol.fbf.y);
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/additional_params/c.wgsl:2:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
