
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
void a(matrix<float16_t, 2, 2> m) {
}

void b(vector<float16_t, 2> v) {
}

void c(float16_t f_1) {
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(u[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]));
}

[numthreads(1, 1, 1)]
void f() {
  a(v_2(0u));
  b(tint_bitcast_to_f16(u[0u].y));
  b(tint_bitcast_to_f16(u[0u].y).yx);
  c(float16_t(f16tof32(u[0u].y)));
  c(tint_bitcast_to_f16(u[0u].y).yx.x);
}

