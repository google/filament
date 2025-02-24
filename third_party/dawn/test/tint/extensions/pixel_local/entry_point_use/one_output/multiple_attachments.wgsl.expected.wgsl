enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn f() -> @location(0) vec4f {
  P.a += 42;
  return vec4f(2);
}
