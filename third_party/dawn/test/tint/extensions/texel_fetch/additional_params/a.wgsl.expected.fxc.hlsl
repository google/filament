SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

struct In {
  @location(0)
  uv : vec4f,
}

fn g(a : f32, b : f32, c : f32) {
}

@fragment
fn f(@builtin(position) pos : vec4f, @color(0) fbf : vec4f, tint_symbol : In) {
  g(pos.x, fbf.x, tint_symbol.uv.x);
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/additional_params/a.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
