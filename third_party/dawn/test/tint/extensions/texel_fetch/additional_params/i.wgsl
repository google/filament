enable chromium_experimental_framebuffer_fetch;

struct In {
  @location(0) a : vec4f,
  @interpolate(flat) @location(1) b : vec4f,
  @color(1) fbf : vec4i,
}

@fragment fn f(in : In) {
  g(in.a.x, in.b.y, in.fbf.x);
}

fn g(a : f32, b : f32, c : i32) {}
