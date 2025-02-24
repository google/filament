// flags: --pixel-local-attachments 0=1 --pixel-local-attachment-formats 0=R32Uint
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
}

var<pixel_local> P : PixelLocal;

fn f0() {
  P.a += 9;
}

fn f1() {
  f0();
  P.a += 8;
}

fn f2() {
  P.a += 7;
  f1();
}

@fragment fn f() -> @location(0) vec4f {
  f2();
  return vec4f(2);
}
