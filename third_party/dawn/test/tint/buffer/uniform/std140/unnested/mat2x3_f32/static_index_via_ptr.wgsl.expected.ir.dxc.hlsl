
cbuffer cbuffer_m : register(b0) {
  uint4 m[2];
};
float2x3 v(uint start_byte_offset) {
  return float2x3(asfloat(m[(start_byte_offset / 16u)].xyz), asfloat(m[((16u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  float2x3 l_m = v(0u);
  float3 l_m_1 = asfloat(m[1u].xyz);
}

