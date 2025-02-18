
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
float16_t a(matrix<float16_t, 2, 4> a_1[4]) {
  return a_1[0u][0u].x;
}

float16_t b(matrix<float16_t, 2, 4> m) {
  return m[0u].x;
}

float16_t c(vector<float16_t, 4> v) {
  return v.x;
}

float16_t d(float16_t f_1) {
  return f_1;
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

matrix<float16_t, 2, 4> v_4(uint start_byte_offset) {
  uint4 v_5 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = u[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 4>(v_6, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

typedef matrix<float16_t, 2, 4> ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  matrix<float16_t, 2, 4> a_2[4] = (matrix<float16_t, 2, 4>[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a_2[v_10] = v_4((start_byte_offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 2, 4> v_11[4] = a_2;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 4> v_12[4] = v_8(0u);
  float16_t v_13 = a(v_12);
  float16_t v_14 = (v_13 + b(v_4(16u)));
  float16_t v_15 = (v_14 + c(tint_bitcast_to_f16(u[1u].xy).ywxz));
  s.Store<float16_t>(0u, (v_15 + d(tint_bitcast_to_f16(u[1u].xy).ywxz.x)));
}

