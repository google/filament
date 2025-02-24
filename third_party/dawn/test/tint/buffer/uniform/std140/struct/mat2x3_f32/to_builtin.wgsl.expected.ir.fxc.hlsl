
cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
float2x3 v(uint start_byte_offset) {
  return float2x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  float3x2 t = transpose(v(272u));
  float l = length(asfloat(u[2u].xyz).zxy);
  float a = abs(asfloat(u[2u].xyz).zxy.x);
}

