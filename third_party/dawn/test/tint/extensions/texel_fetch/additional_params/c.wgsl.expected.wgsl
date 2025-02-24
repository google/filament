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
fn f(in : In) {
  g(in.pos.x, in.uv.x, in.fbf.y);
}
