enable chromium_experimental_framebuffer_fetch;

struct In {
  @location(0)
  uv : vec4f,
}

fn g(a : f32, b : f32, c : f32) {
}

@fragment
fn f(@builtin(position) pos : vec4f, @color(0) fbf : vec4f, in : In) {
  g(pos.x, fbf.x, in.uv.x);
}
