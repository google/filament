
cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
static float2x3 p = float2x3((0.0f).xxx, (0.0f).xxx);
float2x3 v(uint start_byte_offset) {
  return float2x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  p = v(0u);
  p[1u] = asfloat(u[0u].xyz);
  p[1u] = asfloat(u[0u].xyz).zxy;
  p[0u].y = asfloat(u[1u].x);
}

