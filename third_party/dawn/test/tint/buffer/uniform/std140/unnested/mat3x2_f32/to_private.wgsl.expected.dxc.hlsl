cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
static float3x2 p = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

float3x2 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = u[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = u[scalar_offset_2 / 4];
  return float3x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  p = u_load(0u);
  p[1] = asfloat(u[0].xy);
  p[1] = asfloat(u[0].xy).yx;
  p[0][1] = asfloat(u[0].z);
  return;
}
