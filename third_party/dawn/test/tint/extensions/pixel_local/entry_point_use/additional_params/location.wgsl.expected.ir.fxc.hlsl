struct PixelLocal {
  uint a;
  int b;
  float c;
};

struct f_inputs {
  float4 a : TEXCOORD0;
  nointerpolation float4 b : TEXCOORD1;
  float4 pos : SV_Position;
};


static PixelLocal P = (PixelLocal)0;
RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);
RasterizerOrderedTexture2D<int4> pixel_local_b : register(u6);
RasterizerOrderedTexture2D<float4> pixel_local_c : register(u3);
uint tint_f32_to_u32(float value) {
  return (((value <= 4294967040.0f)) ? ((((value >= 0.0f)) ? (uint(value)) : (0u))) : (4294967295u));
}

void f_inner(float4 a, float4 b) {
  uint v = tint_f32_to_u32(a.x);
  uint v_1 = (v + tint_f32_to_u32(b.y));
  P.a = (P.a + v_1);
}

void f(f_inputs inputs) {
  uint2 v_2 = uint2(inputs.pos.xy);
  P.a = pixel_local_a.Load(v_2).x;
  P.b = pixel_local_b.Load(v_2).x;
  P.c = pixel_local_c.Load(v_2).x;
  f_inner(inputs.a, inputs.b);
  pixel_local_a[v_2] = uint4((P.a).xxxx);
  pixel_local_b[v_2] = int4((P.b).xxxx);
  pixel_local_c[v_2] = float4((P.c).xxxx);
}

