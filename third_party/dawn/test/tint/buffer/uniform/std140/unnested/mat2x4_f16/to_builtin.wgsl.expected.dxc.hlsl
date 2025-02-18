cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};

matrix<float16_t, 2, 4> u_load(uint offset) {
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
  return matrix<float16_t, 2, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]));
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> t = transpose(u_load(0u));
  uint2 ubo_load_4 = u[0].zw;
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  float16_t l = length(vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]));
  uint2 ubo_load_5 = u[0].xy;
  vector<float16_t, 2> ubo_load_5_xz = vector<float16_t, 2>(f16tof32(ubo_load_5 & 0xFFFF));
  vector<float16_t, 2> ubo_load_5_yw = vector<float16_t, 2>(f16tof32(ubo_load_5 >> 16));
  float16_t a = abs(vector<float16_t, 4>(ubo_load_5_xz[0], ubo_load_5_yw[0], ubo_load_5_xz[1], ubo_load_5_yw[1]).ywxz.x);
  return;
}
