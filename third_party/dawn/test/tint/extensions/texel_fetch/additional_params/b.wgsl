enable chromium_experimental_framebuffer_fetch;

fn g(a : f32, b : f32, c : f32) {}

@fragment fn f(@builtin(position) pos : vec4f, @location(0) uv : vec4f, @color(0) fbf : vec4f) {
  g(pos.x, uv.x, fbf.x);
}
