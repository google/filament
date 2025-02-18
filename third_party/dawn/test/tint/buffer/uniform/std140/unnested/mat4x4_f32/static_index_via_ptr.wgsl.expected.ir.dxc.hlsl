
cbuffer cbuffer_m : register(b0) {
  uint4 m[4];
};
float4x4 v(uint start_byte_offset) {
  return float4x4(asfloat(m[(start_byte_offset / 16u)]), asfloat(m[((16u + start_byte_offset) / 16u)]), asfloat(m[((32u + start_byte_offset) / 16u)]), asfloat(m[((48u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  float4x4 l_m = v(0u);
  float4 l_m_1 = asfloat(m[1u]);
}

