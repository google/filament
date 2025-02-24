enable chromium_experimental_framebuffer_fetch;

@fragment fn f(@color(0) fbf : vec4f) -> @location(0) vec4f {
  return fbf;
}
