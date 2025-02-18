enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

@fragment
fn f() {
  P.a += 42;
}
