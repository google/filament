cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);

[numthreads(1, 1, 1)]
void main() {
  uint ubo_load = u[0].x;
  vector<float16_t, 2> x = vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16)));
  s.Store<vector<float16_t, 2> >(0u, x);
  return;
}
