enable chromium_experimental_framebuffer_fetch;

@fragment
fn f(@color(3) fbf : vec4f, @builtin(position) pos : vec4f) {
  g(fbf.w, pos.x);
}

fn g(a : f32, b : f32) {
}
