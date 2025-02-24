enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @location(0)
  uv : vec4f,
}

@fragment
fn f(@builtin(position) pos : vec4f, in : In) {
  P.a += (u32(pos.x) + u32(in.uv.x));
}
