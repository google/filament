// flags: --pixel-local-attachments 0=1,1=6,2=3 --pixel-local-attachment-formats 0=R32Uint,1=R32Sint,2=R32Float
enable chromium_experimental_pixel_local;

struct PixelLocal {
  a : u32,
  b : i32,
  c : f32,
}

var<pixel_local> P : PixelLocal;

struct In {
  @builtin(position) pos : vec4f,
  @builtin(front_facing) ff : bool,
  @builtin(sample_index) si : u32,
}

@fragment fn f(in : In) {
  P.a += u32(in.pos.x);
}
