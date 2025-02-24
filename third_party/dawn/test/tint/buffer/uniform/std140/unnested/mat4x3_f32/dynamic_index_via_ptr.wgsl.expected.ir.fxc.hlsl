
cbuffer cbuffer_m : register(b0) {
  uint4 m[4];
};
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

float4x3 v(uint start_byte_offset) {
  return float4x3(asfloat(m[(start_byte_offset / 16u)].xyz), asfloat(m[((16u + start_byte_offset) / 16u)].xyz), asfloat(m[((32u + start_byte_offset) / 16u)].xyz), asfloat(m[((48u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  uint v_1 = (16u * min(uint(i()), 3u));
  float4x3 l_m = v(0u);
  float3 l_m_i = asfloat(m[(v_1 / 16u)].xyz);
}

