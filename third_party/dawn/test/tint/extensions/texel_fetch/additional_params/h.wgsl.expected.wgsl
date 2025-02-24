enable chromium_experimental_framebuffer_fetch;

struct FBF {
  @color(1)
  c1 : vec4f,
  @color(3)
  c3 : vec4i,
}

@fragment
fn f(@invariant @builtin(position) pos : vec4f, fbf : FBF) {
  g(fbf.c1.x, pos.y, fbf.c3.z);
}

fn g(a : f32, b : f32, c : i32) {
}
