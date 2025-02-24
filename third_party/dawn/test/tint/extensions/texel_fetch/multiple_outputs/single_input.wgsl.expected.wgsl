enable chromium_experimental_framebuffer_fetch;

struct Out {
  @location(0)
  x : vec4f,
  @location(2)
  y : vec4f,
  @location(3)
  z : vec4f,
}

@fragment
fn f(@color(0) fbf : vec4f) -> Out {
  return Out(vec4f(10), fbf, vec4f(30));
}
