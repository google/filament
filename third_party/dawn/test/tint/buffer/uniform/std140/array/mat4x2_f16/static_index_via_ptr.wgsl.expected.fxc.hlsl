SKIP: INVALID

cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);

matrix<float16_t, 4, 2> a_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = a[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = a[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = a[scalar_offset_2 / 4][scalar_offset_2 % 4];
  const uint scalar_offset_3 = ((offset + 12u)) / 4;
  uint ubo_load_3 = a[scalar_offset_3 / 4][scalar_offset_3 % 4];
  return matrix<float16_t, 4, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))));
}

typedef matrix<float16_t, 4, 2> a_load_ret[4];
a_load_ret a_load(uint offset) {
  matrix<float16_t, 4, 2> arr[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = a_load_1((offset + (i * 16u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> l_a[4] = a_load(0u);
  matrix<float16_t, 4, 2> l_a_i = a_load_1(32u);
  uint ubo_load_4 = a[2].y;
  vector<float16_t, 2> l_a_i_i = vector<float16_t, 2>(float16_t(f16tof32(ubo_load_4 & 0xFFFF)), float16_t(f16tof32(ubo_load_4 >> 16)));
  s.Store<float16_t>(0u, (((float16_t(f16tof32(((a[2].y) & 0xFFFF))) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x));
  return;
}
FXC validation failure:
<scrubbed_path>(6,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
