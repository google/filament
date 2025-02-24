enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> V : PixelLocal;

@fragment
fn f() {
  let p = &(V);
  (*(p)).a = 42;
}
