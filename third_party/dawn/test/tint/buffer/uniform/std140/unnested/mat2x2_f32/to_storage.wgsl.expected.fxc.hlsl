cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);

void s_store(uint offset, float2x2 value) {
  s.Store2((offset + 0u), asuint(value[0u]));
  s.Store2((offset + 8u), asuint(value[1u]));
}

float2x2 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = u[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  s_store(0u, u_load(0u));
  s.Store2(8u, asuint(asfloat(u[0].xy)));
  s.Store2(8u, asuint(asfloat(u[0].xy).yx));
  s.Store(4u, asuint(asfloat(u[0].z)));
  return;
}
