enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn f(@location(0) a : vec4f, @interpolate(flat) @location(1) b : vec4f) {
  P.a += (u32(a.x) + u32(b.y));
}
