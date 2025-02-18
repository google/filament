
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
float4x3 v(uint start_byte_offset) {
  return float4x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz), asfloat(u[((32u + start_byte_offset) / 16u)].xyz), asfloat(u[((48u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  float3x4 t = transpose(v(0u));
  float l = length(asfloat(u[1u].xyz));
  float a = abs(asfloat(u[0u].xyz).zxy.x);
}

