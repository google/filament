SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

struct In {
  @invariant @builtin(position)
  pos : vec4f,
}

fn g(a : f32, b : f32) {
}

@fragment
fn f(tint_symbol : In, @color(2) fbf : vec4f) {
  g(tint_symbol.pos.x, fbf.g);
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/additional_params/f.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
