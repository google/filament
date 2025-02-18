enable chromium_experimental_framebuffer_fetch;

struct In {
  @invariant @builtin(position)
  pos : vec4f,
}

fn g(a : f32, b : f32) {
}

@fragment
fn f(in : In, @color(2) fbf : vec4f) {
  g(in.pos.x, fbf.g);
}
