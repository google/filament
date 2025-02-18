cbuffer cbuffer_m : register(b0) {
  uint4 m[4];
};

float4x4 m_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(m[scalar_offset / 4]), asfloat(m[scalar_offset_1 / 4]), asfloat(m[scalar_offset_2 / 4]), asfloat(m[scalar_offset_3 / 4]));
}

[numthreads(1, 1, 1)]
void f() {
  float4x4 l_m = m_load(0u);
  float4 l_m_1 = asfloat(m[1]);
  return;
}
