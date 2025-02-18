cbuffer cbuffer_u : register(b0) {
  uint4 u[12];
};
RWByteAddressBuffer s : register(u1);

float a(float3x4 a_1[4]) {
  return a_1[0][0].x;
}

float b(float3x4 m) {
  return m[0].x;
}

float c(float4 v) {
  return v.x;
}

float d(float f_1) {
  return f_1;
}

float3x4 u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]), asfloat(u[scalar_offset_2 / 4]));
}

typedef float3x4 u_load_ret[4];
u_load_ret u_load(uint offset) {
  float3x4 arr[4] = (float3x4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 48u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  float tint_symbol = a(u_load(0u));
  float tint_symbol_1 = b(u_load_1(48u));
  float tint_symbol_2 = c(asfloat(u[3]).ywxz);
  float tint_symbol_3 = d(asfloat(u[3]).ywxz.x);
  s.Store(0u, asuint((((tint_symbol + tint_symbol_1) + tint_symbol_2) + tint_symbol_3)));
  return;
}
