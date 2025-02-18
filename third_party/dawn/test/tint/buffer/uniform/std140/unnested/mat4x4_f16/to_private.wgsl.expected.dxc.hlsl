cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
static matrix<float16_t, 4, 4> p = matrix<float16_t, 4, 4>(float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h), float16_t(0.0h));

matrix<float16_t, 4, 4> u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = u[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  vector<float16_t, 2> ubo_load_2_yw = vector<float16_t, 2>(f16tof32(ubo_load_2 >> 16));
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = u[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_7 = u[scalar_offset_3 / 4];
  uint2 ubo_load_6 = ((scalar_offset_3 & 2) ? ubo_load_7.zw : ubo_load_7.xy);
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  vector<float16_t, 2> ubo_load_6_yw = vector<float16_t, 2>(f16tof32(ubo_load_6 >> 16));
  return matrix<float16_t, 4, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]), vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]), vector<float16_t, 4>(ubo_load_6_xz[0], ubo_load_6_yw[0], ubo_load_6_xz[1], ubo_load_6_yw[1]));
}

[numthreads(1, 1, 1)]
void f() {
  p = u_load(0u);
  uint2 ubo_load_8 = u[0].xy;
  vector<float16_t, 2> ubo_load_8_xz = vector<float16_t, 2>(f16tof32(ubo_load_8 & 0xFFFF));
  vector<float16_t, 2> ubo_load_8_yw = vector<float16_t, 2>(f16tof32(ubo_load_8 >> 16));
  p[1] = vector<float16_t, 4>(ubo_load_8_xz[0], ubo_load_8_yw[0], ubo_load_8_xz[1], ubo_load_8_yw[1]);
  uint2 ubo_load_9 = u[0].xy;
  vector<float16_t, 2> ubo_load_9_xz = vector<float16_t, 2>(f16tof32(ubo_load_9 & 0xFFFF));
  vector<float16_t, 2> ubo_load_9_yw = vector<float16_t, 2>(f16tof32(ubo_load_9 >> 16));
  p[1] = vector<float16_t, 4>(ubo_load_9_xz[0], ubo_load_9_yw[0], ubo_load_9_xz[1], ubo_load_9_yw[1]).ywxz;
  p[0][1] = float16_t(f16tof32(((u[0].z) & 0xFFFF)));
  return;
}
