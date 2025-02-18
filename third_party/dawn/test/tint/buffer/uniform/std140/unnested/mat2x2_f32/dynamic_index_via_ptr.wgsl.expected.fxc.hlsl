cbuffer cbuffer_m : register(b0) {
  uint4 m[1];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

float2x2 m_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = m[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = m[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  int p_m_i_save = i();
  float2x2 l_m = m_load(0u);
  const uint scalar_offset_2 = ((8u * min(uint(p_m_i_save), 1u))) / 4;
  uint4 ubo_load_2 = m[scalar_offset_2 / 4];
  float2 l_m_i = asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy));
  return;
}
