
cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
float2x4 v(uint start_byte_offset) {
  return float2x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 t = transpose(v(272u));
  float l = length(asfloat(u[2u]).ywxz);
  float a = abs(asfloat(u[2u]).ywxz.x);
}

