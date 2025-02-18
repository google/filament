enable chromium_experimental_framebuffer_fetch;

struct In {
  @color(3)
  fbf : vec4i,
  @builtin(position)
  pos : vec4f,
}

@fragment
fn f(in : In) {
  g(in.fbf.w, in.pos.x);
}

fn g(a : i32, b : f32) {
}
