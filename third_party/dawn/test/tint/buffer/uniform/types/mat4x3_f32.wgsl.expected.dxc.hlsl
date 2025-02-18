cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);

float4x3 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz), asfloat(u[scalar_offset_2 / 4].xyz), asfloat(u[scalar_offset_3 / 4].xyz));
}

void s_store(uint offset, float4x3 value) {
  s.Store3((offset + 0u), asuint(value[0u]));
  s.Store3((offset + 16u), asuint(value[1u]));
  s.Store3((offset + 32u), asuint(value[2u]));
  s.Store3((offset + 48u), asuint(value[3u]));
}

[numthreads(1, 1, 1)]
void main() {
  float4x3 x = u_load(0u);
  s_store(0u, x);
  return;
}
