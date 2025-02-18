SKIP: INVALID

cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);

float16_t a(matrix<float16_t, 4, 2> a_1[4]) {
  return a_1[0][0].x;
}

float16_t b(matrix<float16_t, 4, 2> m) {
  return m[0].x;
}

float16_t c(vector<float16_t, 2> v) {
  return v.x;
}

float16_t d(float16_t f_1) {
  return f_1;
}

matrix<float16_t, 4, 2> u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = u[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = u[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = u[scalar_offset_2 / 4][scalar_offset_2 % 4];
  const uint scalar_offset_3 = ((offset + 12u)) / 4;
  uint ubo_load_3 = u[scalar_offset_3 / 4][scalar_offset_3 % 4];
  return matrix<float16_t, 4, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))));
}

typedef matrix<float16_t, 4, 2> u_load_ret[4];
u_load_ret u_load(uint offset) {
  matrix<float16_t, 4, 2> arr[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 16u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  float16_t tint_symbol = a(u_load(0u));
  float16_t tint_symbol_1 = b(u_load_1(16u));
  uint ubo_load_4 = u[1].x;
  float16_t tint_symbol_2 = c(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_4 & 0xFFFF)), float16_t(f16tof32(ubo_load_4 >> 16))).yx);
  uint ubo_load_5 = u[1].x;
  float16_t tint_symbol_3 = d(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_5 & 0xFFFF)), float16_t(f16tof32(ubo_load_5 >> 16))).yx.x);
  s.Store<float16_t>(0u, (((tint_symbol + tint_symbol_1) + tint_symbol_2) + tint_symbol_3));
  return;
}
FXC validation failure:
<scrubbed_path>(6,1-9): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
