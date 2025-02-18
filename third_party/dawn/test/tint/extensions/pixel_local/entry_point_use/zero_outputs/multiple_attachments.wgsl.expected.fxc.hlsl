RasterizerOrderedTexture2D<uint4> pixel_local_a : register(u1);
RasterizerOrderedTexture2D<int4> pixel_local_b : register(u6);
RasterizerOrderedTexture2D<float4> pixel_local_c : register(u3);

struct PixelLocal {
  uint a;
  int b;
  float c;
};

static PixelLocal P = (PixelLocal)0;

void load_from_pixel_local_storage(float4 my_input) {
  uint2 rov_texcoord = uint2(my_input.xy);
  P.a = pixel_local_a.Load(rov_texcoord).x;
  P.b = pixel_local_b.Load(rov_texcoord).x;
  P.c = pixel_local_c.Load(rov_texcoord).x;
}

void store_into_pixel_local_storage(float4 my_input) {
  uint2 rov_texcoord = uint2(my_input.xy);
  pixel_local_a[rov_texcoord] = uint4((P.a).xxxx);
  pixel_local_b[rov_texcoord] = int4((P.b).xxxx);
  pixel_local_c[rov_texcoord] = float4((P.c).xxxx);
}

struct tint_symbol_1 {
  float4 my_pos : SV_Position;
};

void f_inner() {
  P.a = (P.a + 42u);
}

void f_inner_1(float4 my_pos) {
  float4 hlsl_sv_position = my_pos;
  load_from_pixel_local_storage(hlsl_sv_position);
  f_inner();
  store_into_pixel_local_storage(hlsl_sv_position);
}

void f(tint_symbol_1 tint_symbol) {
  f_inner_1(float4(tint_symbol.my_pos.xyz, (1.0f / tint_symbol.my_pos.w)));
  return;
}
