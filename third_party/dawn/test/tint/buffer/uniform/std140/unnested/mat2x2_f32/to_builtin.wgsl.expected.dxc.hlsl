cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};

float2x2 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = u[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 t = transpose(u_load(0u));
  float l = length(asfloat(u[0].zw));
  float a = abs(asfloat(u[0].xy).yx.x);
  return;
}
