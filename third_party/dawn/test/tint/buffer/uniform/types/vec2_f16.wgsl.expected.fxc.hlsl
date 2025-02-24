SKIP: INVALID

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
FXC validation failure:
<scrubbed_path>(9,10-18): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(10,3-9): error X3018: invalid subscript 'Store'


tint executable returned error: exit status 1
