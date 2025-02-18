SKIP: INVALID


enable chromium_experimental_framebuffer_fetch;

struct In {
  @color(3)
  fbf : vec4i,
  @builtin(position)
  pos : vec4f,
}

@fragment
fn f(tint_symbol : In) {
  g(tint_symbol.fbf.w, tint_symbol.pos.x);
}

fn g(a : i32, b : f32) {
}

Failed to generate: <dawn>/test/tint/extensions/texel_fetch/additional_params/e.wgsl:1:8 error: HLSL backend does not support extension 'chromium_experimental_framebuffer_fetch'
enable chromium_experimental_framebuffer_fetch;
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


tint executable returned error: exit status 1
