enable chromium_experimental_framebuffer_fetch;

@fragment fn f(@color(0) fbf : vec4f) {
  g(fbf.y);
}

fn g(a : f32) {}
