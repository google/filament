SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
void a(matrix<float16_t, 3, 2> m) {
}

void b(vector<float16_t, 2> v) {
}

void c(float16_t f) {
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 3, 2> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_3.z) : (v_3.x)));
  uint4 v_5 = u[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.z) : (v_5.x)));
  uint4 v_7 = u[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 2>(v_4, v_6, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.z) : (v_7.x))));
}

[numthreads(1, 1, 1)]
void f() {
  a(v_2(0u));
  b(tint_bitcast_to_f16(u[0u].x));
  b(tint_bitcast_to_f16(u[0u].x).yx);
  c(float16_t(f16tof32(u[0u].y)));
  c(tint_bitcast_to_f16(u[0u].x).yx[0u]);
}

FXC validation failure:
<scrubbed_path>(5,15-23): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(8,15-23): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(11,8-16): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
