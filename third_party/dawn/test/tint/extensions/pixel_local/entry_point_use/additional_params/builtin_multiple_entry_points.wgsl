// flags: --pixel-local-attachments 0=1,1=6,2=3 --pixel-local-attachment-formats 0=R32Uint,1=R32Sint,2=R32Float
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

@fragment fn f(@builtin(position) pos : vec4f) {
  P.a += u32(pos.x);
}

@fragment fn f2(@builtin(position) pos : vec4f) {
  P.b += i32(pos.x);
}

@fragment fn f3(@builtin(position) pos : vec4f) {
  P.c += pos.x;
}
