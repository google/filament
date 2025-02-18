cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
static float2x3 p = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

float2x3 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  p = u_load(0u);
  p[1] = asfloat(u[0].xyz);
  p[1] = asfloat(u[0].xyz).zxy;
  p[0][1] = asfloat(u[1].x);
  return;
}
