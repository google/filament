//
// f
//
struct PixelLocal {
  uint a;
  int b;
  float c;
};

struct f_inputs {
  float4 pos : SV_Position;
};


static PixelLocal P = (PixelLocal)0;
RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);
RasterizerOrderedTexture2D<int4> pixel_local_b : register(u6);
RasterizerOrderedTexture2D<float4> pixel_local_c : register(u3);
uint tint_f32_to_u32(float value) {
  return (((value <= 4294967040.0f)) ? ((((value >= 0.0f)) ? (uint(value)) : (0u))) : (4294967295u));
}

void f_inner(float4 pos) {
  uint v = tint_f32_to_u32(pos.x);
  P.a = (P.a + v);
}

void f(f_inputs inputs) {
  uint2 v_1 = uint2(inputs.pos.xy);
  P.a = pixel_local_a.Load(v_1).x;
  P.b = pixel_local_b.Load(v_1).x;
  P.c = pixel_local_c.Load(v_1).x;
  f_inner(float4(inputs.pos.xyz, (1.0f / inputs.pos.w)));
  pixel_local_a[v_1] = uint4((P.a).xxxx);
  pixel_local_b[v_1] = int4((P.b).xxxx);
  pixel_local_c[v_1] = float4((P.c).xxxx);
}

//
// f2
//
struct PixelLocal {
  uint a;
  int b;
  float c;
};

struct f2_inputs {
  float4 pos : SV_Position;
};


static PixelLocal P = (PixelLocal)0;
RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);
RasterizerOrderedTexture2D<int4> pixel_local_b : register(u6);
RasterizerOrderedTexture2D<float4> pixel_local_c : register(u3);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

void f2_inner(float4 pos) {
  int v = tint_f32_to_i32(pos.x);
  P.b = (P.b + v);
}

void f2(f2_inputs inputs) {
  uint2 v_1 = uint2(inputs.pos.xy);
  P.a = pixel_local_a.Load(v_1).x;
  P.b = pixel_local_b.Load(v_1).x;
  P.c = pixel_local_c.Load(v_1).x;
  f2_inner(float4(inputs.pos.xyz, (1.0f / inputs.pos.w)));
  pixel_local_a[v_1] = uint4((P.a).xxxx);
  pixel_local_b[v_1] = int4((P.b).xxxx);
  pixel_local_c[v_1] = float4((P.c).xxxx);
}

//
// f3
//
struct PixelLocal {
  uint a;
  int b;
  float c;
};

struct f3_inputs {
  float4 pos : SV_Position;
};


static PixelLocal P = (PixelLocal)0;
RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);
RasterizerOrderedTexture2D<int4> pixel_local_b : register(u6);
RasterizerOrderedTexture2D<float4> pixel_local_c : register(u3);
void f3_inner(float4 pos) {
  P.c = (P.c + pos.x);
}

void f3(f3_inputs inputs) {
  uint2 v = uint2(inputs.pos.xy);
  P.a = pixel_local_a.Load(v).x;
  P.b = pixel_local_b.Load(v).x;
  P.c = pixel_local_c.Load(v).x;
  f3_inner(float4(inputs.pos.xyz, (1.0f / inputs.pos.w)));
  pixel_local_a[v] = uint4((P.a).xxxx);
  pixel_local_b[v] = int4((P.b).xxxx);
  pixel_local_c[v] = float4((P.c).xxxx);
}

