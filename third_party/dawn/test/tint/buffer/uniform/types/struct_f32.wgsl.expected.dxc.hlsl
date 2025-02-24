struct Inner {
  float scalar_f32;
  float3 vec3_f32;
  float2x4 mat2x4_f32;
};
struct S {
  Inner inner;
};

cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);

float2x4 u_load_4(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]));
}

Inner u_load_1(uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 16u)) / 4;
  Inner tint_symbol = {asfloat(u[scalar_offset_2 / 4][scalar_offset_2 % 4]), asfloat(u[scalar_offset_3 / 4].xyz), u_load_4((offset + 32u))};
  return tint_symbol;
}

S u_load(uint offset) {
  S tint_symbol_1 = {u_load_1((offset + 0u))};
  return tint_symbol_1;
}

void s_store_4(uint offset, float2x4 value) {
  s.Store4((offset + 0u), asuint(value[0u]));
  s.Store4((offset + 16u), asuint(value[1u]));
}

void s_store_1(uint offset, Inner value) {
  s.Store((offset + 0u), asuint(value.scalar_f32));
  s.Store3((offset + 16u), asuint(value.vec3_f32));
  s_store_4((offset + 32u), value.mat2x4_f32);
}

void s_store(uint offset, S value) {
  s_store_1((offset + 0u), value.inner);
}

[numthreads(1, 1, 1)]
void main() {
  S x = u_load(0u);
  s_store(0u, x);
  return;
}
