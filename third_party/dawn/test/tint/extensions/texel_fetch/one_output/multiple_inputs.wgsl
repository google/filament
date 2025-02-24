enable chromium_experimental_framebuffer_fetch;

@fragment fn f(@color(1) fbf_1 : vec4f, @color(3) fbf_3 : vec4f) -> @location(0) vec4f {
  return fbf_1 + fbf_3;
}
