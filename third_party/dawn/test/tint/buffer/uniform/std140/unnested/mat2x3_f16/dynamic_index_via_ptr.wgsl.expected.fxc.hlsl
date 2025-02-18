SKIP: INVALID

cbuffer cbuffer_m : register(b0) {
  uint4 m[1];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

matrix<float16_t, 2, 3> m_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = m[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = m[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  return matrix<float16_t, 2, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]));
}

[numthreads(1, 1, 1)]
void f() {
  int p_m_i_save = i();
  matrix<float16_t, 2, 3> l_m = m_load(0u);
  const uint scalar_offset_2 = ((8u * uint(p_m_i_save))) / 4;
  uint4 ubo_load_5 = m[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  vector<float16_t, 3> l_m_i = vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]);
  return;
}
FXC validation failure:
<scrubbed_path>(11,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
