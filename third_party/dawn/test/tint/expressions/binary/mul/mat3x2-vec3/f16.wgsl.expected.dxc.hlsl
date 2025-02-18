cbuffer cbuffer_data : register(b0) {
  uint4 data[2];
};

matrix<float16_t, 3, 2> data_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = data[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = data[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = data[scalar_offset_2 / 4][scalar_offset_2 % 4];
  return matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))));
}

void main() {
  uint2 ubo_load_3 = data[1].xy;
  vector<float16_t, 2> ubo_load_3_xz = vector<float16_t, 2>(f16tof32(ubo_load_3 & 0xFFFF));
  float16_t ubo_load_3_y = f16tof32(ubo_load_3[0] >> 16);
  vector<float16_t, 2> x = mul(vector<float16_t, 3>(ubo_load_3_xz[0], ubo_load_3_y, ubo_load_3_xz[1]), data_load(0u));
  return;
}
