cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);

matrix<float16_t, 4, 2> u_load(uint offset) {
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

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 4> t = transpose(u_load(32u));
  uint ubo_load_4 = u[0].y;
  float16_t l = length(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_4 & 0xFFFF)), float16_t(f16tof32(ubo_load_4 >> 16))).yx);
  uint ubo_load_5 = u[0].y;
  float16_t a = abs(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_5 & 0xFFFF)), float16_t(f16tof32(ubo_load_5 >> 16))).yx.x);
  s.Store<float16_t>(0u, ((t[0].x + float16_t(l)) + float16_t(a)));
  return;
}
