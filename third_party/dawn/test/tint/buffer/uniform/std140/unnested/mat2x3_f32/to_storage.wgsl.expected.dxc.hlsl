cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
RWByteAddressBuffer s : register(u1);

void s_store(uint offset, float2x3 value) {
  s.Store3((offset + 0u), asuint(value[0u]));
  s.Store3((offset + 16u), asuint(value[1u]));
}

float2x3 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  s_store(0u, u_load(0u));
  s.Store3(16u, asuint(asfloat(u[0].xyz)));
  s.Store3(16u, asuint(asfloat(u[0].xyz).zxy));
  s.Store(4u, asuint(asfloat(u[1].x)));
  return;
}
