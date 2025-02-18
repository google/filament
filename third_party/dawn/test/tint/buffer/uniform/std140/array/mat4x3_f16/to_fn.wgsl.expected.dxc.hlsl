cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);

float16_t a(matrix<float16_t, 4, 3> a_1[4]) {
  return a_1[0][0].x;
}

float16_t b(matrix<float16_t, 4, 3> m) {
  return m[0].x;
}

float16_t c(vector<float16_t, 3> v) {
  return v.x;
}

float16_t d(float16_t f_1) {
  return f_1;
}

matrix<float16_t, 4, 3> u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = u[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = u[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_7 = u[scalar_offset_3 / 4];
  uint2 ubo_load_6 = ((scalar_offset_3 & 2) ? ubo_load_7.zw : ubo_load_7.xy);
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  float16_t ubo_load_6_y = f16tof32(ubo_load_6[0] >> 16);
  return matrix<float16_t, 4, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]), vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]), vector<float16_t, 3>(ubo_load_6_xz[0], ubo_load_6_y, ubo_load_6_xz[1]));
}

typedef matrix<float16_t, 4, 3> u_load_ret[4];
u_load_ret u_load(uint offset) {
  matrix<float16_t, 4, 3> arr[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  float16_t tint_symbol = a(u_load(0u));
  float16_t tint_symbol_1 = b(u_load_1(32u));
  uint2 ubo_load_8 = u[2].xy;
  vector<float16_t, 2> ubo_load_8_xz = vector<float16_t, 2>(f16tof32(ubo_load_8 & 0xFFFF));
  float16_t ubo_load_8_y = f16tof32(ubo_load_8[0] >> 16);
  float16_t tint_symbol_2 = c(vector<float16_t, 3>(ubo_load_8_xz[0], ubo_load_8_y, ubo_load_8_xz[1]).zxy);
  uint2 ubo_load_9 = u[2].xy;
  vector<float16_t, 2> ubo_load_9_xz = vector<float16_t, 2>(f16tof32(ubo_load_9 & 0xFFFF));
  float16_t ubo_load_9_y = f16tof32(ubo_load_9[0] >> 16);
  float16_t tint_symbol_3 = d(vector<float16_t, 3>(ubo_load_9_xz[0], ubo_load_9_y, ubo_load_9_xz[1]).zxy.x);
  s.Store<float16_t>(0u, (((tint_symbol + tint_symbol_1) + tint_symbol_2) + tint_symbol_3));
  return;
}
