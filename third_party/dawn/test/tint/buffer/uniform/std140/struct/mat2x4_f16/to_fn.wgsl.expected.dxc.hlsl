struct S {
  int before;
  matrix<float16_t, 2, 4> m;
  int after;
};

cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};

void a(S a_1[4]) {
}

void b(S s) {
}

void c(matrix<float16_t, 2, 4> m) {
}

void d(vector<float16_t, 4> v) {
}

void e(float16_t f_1) {
}

matrix<float16_t, 2, 4> u_load_3(uint offset) {
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

S u_load_1(uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 64u)) / 4;
  S tint_symbol = {asint(u[scalar_offset_2 / 4][scalar_offset_2 % 4]), u_load_3((offset + 8u)), asint(u[scalar_offset_3 / 4][scalar_offset_3 % 4])};
  return tint_symbol;
}

typedef S u_load_ret[4];
u_load_ret u_load(uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 128u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  a(u_load(0u));
  b(u_load_1(256u));
  c(u_load_3(264u));
  uint2 ubo_load_4 = u[1].xy;
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  d(vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]).ywxz);
  uint2 ubo_load_5 = u[1].xy;
  vector<float16_t, 2> ubo_load_5_xz = vector<float16_t, 2>(f16tof32(ubo_load_5 & 0xFFFF));
  vector<float16_t, 2> ubo_load_5_yw = vector<float16_t, 2>(f16tof32(ubo_load_5 >> 16));
  e(vector<float16_t, 4>(ubo_load_5_xz[0], ubo_load_5_yw[0], ubo_load_5_xz[1], ubo_load_5_yw[1]).ywxz.x);
  return;
}
