
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
float16_t a(matrix<float16_t, 4, 2> a_1[4]) {
  return a_1[0u][0u].x;
}

float16_t b(matrix<float16_t, 4, 2> m) {
  return m[0u].x;
}

float16_t c(vector<float16_t, 2> v) {
  return v.x;
}

float16_t d(float16_t f_1) {
  return f_1;
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 4, 2>(v_3, v_4, v_5, tint_bitcast_to_f16(u[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a_2[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a_2[v_8] = v_2((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_9[4] = a_2;
  return v_9;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> v_10[4] = v_6(0u);
  float16_t v_11 = a(v_10);
  float16_t v_12 = (v_11 + b(v_2(16u)));
  float16_t v_13 = (v_12 + c(tint_bitcast_to_f16(u[1u].x).yx));
  s.Store<float16_t>(0u, (v_13 + d(tint_bitcast_to_f16(u[1u].x).yx.x)));
}

