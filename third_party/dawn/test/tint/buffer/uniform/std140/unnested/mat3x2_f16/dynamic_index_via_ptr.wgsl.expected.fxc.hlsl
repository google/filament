SKIP: INVALID

cbuffer cbuffer_m : register(b0) {
  uint4 m[1];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

matrix<float16_t, 3, 2> m_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = m[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = m[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = m[scalar_offset_2 / 4][scalar_offset_2 % 4];
  return matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))));
}

[numthreads(1, 1, 1)]
void f() {
  int p_m_i_save = i();
  matrix<float16_t, 3, 2> l_m = m_load(0u);
  const uint scalar_offset_3 = ((4u * uint(p_m_i_save))) / 4;
  uint ubo_load_3 = m[scalar_offset_3 / 4][scalar_offset_3 % 4];
  vector<float16_t, 2> l_m_i = vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16)));
  return;
}
FXC validation failure:
<scrubbed_path>(11,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
