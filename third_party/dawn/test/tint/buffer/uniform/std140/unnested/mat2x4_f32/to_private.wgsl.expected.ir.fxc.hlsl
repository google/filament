
cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
static float2x4 p = float2x4((0.0f).xxxx, (0.0f).xxxx);
float2x4 v(uint start_byte_offset) {
  return float2x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  p = v(0u);
  p[1u] = asfloat(u[0u]);
  p[1u] = asfloat(u[0u]).ywxz;
  p[0u].y = asfloat(u[1u].x);
}

