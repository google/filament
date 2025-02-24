
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

[numthreads(1, 1, 1)]
void main() {
  vector<float16_t, 2> x = tint_bitcast_to_f16(u[0u].x);
  s.Store<vector<float16_t, 2> >(0u, x);
}

