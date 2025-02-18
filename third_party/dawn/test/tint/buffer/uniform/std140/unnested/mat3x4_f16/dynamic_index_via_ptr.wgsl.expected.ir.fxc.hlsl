SKIP: INVALID


cbuffer cbuffer_m : register(b0) {
  uint4 m[2];
};
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

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

matrix<float16_t, 3, 4> v_4(uint start_byte_offset) {
  vector<float16_t, 4> v_5 = tint_bitcast_to_f16(m[(start_byte_offset / 16u)]);
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16(m[((8u + start_byte_offset) / 16u)]);
  return matrix<float16_t, 3, 4>(v_5, v_6, tint_bitcast_to_f16(m[((16u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  uint v_7 = (8u * uint(i()));
  matrix<float16_t, 3, 4> l_m = v_4(0u);
  vector<float16_t, 4> l_m_i = tint_bitcast_to_f16(m[(v_7 / 16u)]);
}

FXC validation failure:
<scrubbed_path>(11,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
