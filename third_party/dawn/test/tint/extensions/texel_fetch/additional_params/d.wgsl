enable chromium_experimental_framebuffer_fetch;

fn g(a : i32, b : f32, c : f32, d : u32) {}

struct In {
  @builtin(position) pos : vec4f,
}

@fragment fn f(@color(2) fbf_2 : vec4i, in : In, @location(0) uv : vec4f, @color(0) fbf_0 : vec4u) {
  g(fbf_2.z, in.pos.x, uv.x, fbf_0.y);
}
