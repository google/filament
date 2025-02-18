SKIP: INVALID

cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

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
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = a_load_1((offset + (i_1 * 16u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  int p_a_i_save = i();
  int p_a_i_i_save = i();
  matrix<float16_t, 4, 2> l_a[4] = a_load(0u);
  matrix<float16_t, 4, 2> l_a_i = a_load_1((16u * uint(p_a_i_save)));
  const uint scalar_offset_4 = (((16u * uint(p_a_i_save)) + (4u * uint(p_a_i_i_save)))) / 4;
  uint ubo_load_4 = a[scalar_offset_4 / 4][scalar_offset_4 % 4];
  vector<float16_t, 2> l_a_i_i = vector<float16_t, 2>(float16_t(f16tof32(ubo_load_4 & 0xFFFF)), float16_t(f16tof32(ubo_load_4 >> 16)));
  const uint scalar_offset_bytes = (((16u * uint(p_a_i_save)) + (4u * uint(p_a_i_i_save))));
  const uint scalar_offset_index = scalar_offset_bytes / 4;
  s.Store<float16_t>(0u, (((float16_t(f16tof32(((a[scalar_offset_index / 4][scalar_offset_index % 4] >> (scalar_offset_bytes % 4 == 0 ? 0 : 16)) & 0xFFFF))) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x));
  return;
}
FXC validation failure:
<scrubbed_path>(12,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
