SKIP: INVALID

cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);

void s_store_1(uint offset, matrix<float16_t, 4, 2> value) {
  s.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
  s.Store<vector<float16_t, 2> >((offset + 12u), value[3u]);
}

void s_store(uint offset, matrix<float16_t, 4, 2> value[4]) {
  matrix<float16_t, 4, 2> array_1[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      s_store_1((offset + (i * 16u)), array_1[i]);
    }
  }
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
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = u_load_1((offset + (i_1 * 16u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  s_store(0u, u_load(0u));
  s_store_1(16u, u_load_1(32u));
  uint ubo_load_4 = u[0].y;
  s.Store<vector<float16_t, 2> >(16u, vector<float16_t, 2>(float16_t(f16tof32(ubo_load_4 & 0xFFFF)), float16_t(f16tof32(ubo_load_4 >> 16))).yx);
  s.Store<float16_t>(16u, float16_t(f16tof32(((u[0].y) & 0xFFFF))));
  return;
}
FXC validation failure:
<scrubbed_path>(6,36-44): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(7,3-9): error X3018: invalid subscript 'Store'


tint executable returned error: exit status 1
