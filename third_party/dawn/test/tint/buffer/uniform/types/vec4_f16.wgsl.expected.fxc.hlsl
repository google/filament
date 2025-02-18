SKIP: INVALID

cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);

[numthreads(1, 1, 1)]
void main() {
  uint2 ubo_load = u[0].xy;
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  vector<float16_t, 4> x = vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]);
  s.Store<vector<float16_t, 4> >(0u, x);
  return;
}
FXC validation failure:
<scrubbed_path>(9,10-18): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(10,10-18): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(11,10-18): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(12,3-9): error X3018: invalid subscript 'Store'


tint executable returned error: exit status 1
