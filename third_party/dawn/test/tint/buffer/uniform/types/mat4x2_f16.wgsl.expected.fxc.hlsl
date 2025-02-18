SKIP: INVALID

cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
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

void s_store(uint offset, matrix<float16_t, 4, 2> value) {
  s.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
  s.Store<vector<float16_t, 2> >((offset + 12u), value[3u]);
}

[numthreads(1, 1, 1)]
void main() {
  matrix<float16_t, 4, 2> x = u_load(0u);
  s_store(0u, x);
  return;
}
FXC validation failure:
<scrubbed_path>(6,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
