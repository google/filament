cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};

void a(float3x2 m) {
}

void b(float2 v) {
}

void c(float f_1) {
}

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
  a(u_load(0u));
  b(asfloat(u[0].zw));
  b(asfloat(u[0].zw).yx);
  c(asfloat(u[0].z));
  c(asfloat(u[0].zw).yx.x);
  return;
}
