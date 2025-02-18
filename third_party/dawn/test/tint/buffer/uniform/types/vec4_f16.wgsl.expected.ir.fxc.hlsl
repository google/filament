SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 4> tint_bitcast_to_f16(uint4 src) {
  uint4 v = src;
  uint4 mask = (65535u).xxxx;
  uint4 shift = (16u).xxxx;
  float4 t_low = f16tof32((v & mask));
  float4 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

[numthreads(1, 1, 1)]
void main() {
  vector<float16_t, 4> x = tint_bitcast_to_f16(u[0u]);
  s.Store<vector<float16_t, 4> >(0u, x);
}

FXC validation failure:
<scrubbed_path>(6,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
