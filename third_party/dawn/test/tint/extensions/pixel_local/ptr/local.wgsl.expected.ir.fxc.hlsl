struct PixelLocal {
  uint a;
};

struct f_inputs {
  float4 pos : SV_Position;
};


static PixelLocal V = (PixelLocal)0;
RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);
void f_inner() {
  V.a = 42u;
}

void f(f_inputs inputs) {
  uint2 v = uint2(inputs.pos.xy);
  V.a = pixel_local_a.Load(v).x;
  f_inner();
  pixel_local_a[v] = uint4((V.a).xxxx);
}

