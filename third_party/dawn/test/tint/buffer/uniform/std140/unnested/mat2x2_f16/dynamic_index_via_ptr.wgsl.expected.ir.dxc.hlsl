
cbuffer cbuffer_m : register(b0) {
  uint4 m[1];
};
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(m[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(m[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]));
}

[numthreads(1, 1, 1)]
void f() {
  uint v_4 = (4u * min(uint(i()), 1u));
  matrix<float16_t, 2, 2> l_m = v_2(0u);
  vector<float16_t, 2> l_m_i = tint_bitcast_to_f16(m[(v_4 / 16u)][((v_4 % 16u) / 4u)]);
}

