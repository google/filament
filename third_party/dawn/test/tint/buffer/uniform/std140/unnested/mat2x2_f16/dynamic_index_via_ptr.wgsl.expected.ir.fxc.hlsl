SKIP: INVALID


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
  uint4 v_3 = m[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_3.z) : (v_3.x)));
  uint4 v_5 = m[((4u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 2>(v_4, tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.z) : (v_5.x))));
}

[numthreads(1, 1, 1)]
void f() {
  uint v_6 = (4u * uint(i()));
  matrix<float16_t, 2, 2> l_m = v_2(0u);
  uint4 v_7 = m[(v_6 / 16u)];
  vector<float16_t, 2> l_m_i = tint_bitcast_to_f16((((((v_6 % 16u) / 4u) == 2u)) ? (v_7.z) : (v_7.x)));
}

FXC validation failure:
<scrubbed_path>(11,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
