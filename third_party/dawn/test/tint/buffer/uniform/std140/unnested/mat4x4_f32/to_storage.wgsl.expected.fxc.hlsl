cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);

void s_store(uint offset, float4x4 value) {
  s.Store4((offset + 0u), asuint(value[0u]));
  s.Store4((offset + 16u), asuint(value[1u]));
  s.Store4((offset + 32u), asuint(value[2u]));
  s.Store4((offset + 48u), asuint(value[3u]));
}

float4x4 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]), asfloat(u[scalar_offset_2 / 4]), asfloat(u[scalar_offset_3 / 4]));
}

[numthreads(1, 1, 1)]
void f() {
  s_store(0u, u_load(0u));
  s.Store4(16u, asuint(asfloat(u[0])));
  s.Store4(16u, asuint(asfloat(u[0]).ywxz));
  s.Store(4u, asuint(asfloat(u[1].x)));
  return;
}
