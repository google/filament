SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
float16_t a(matrix<float16_t, 4, 2> a_1[4]) {
  return a_1[int(0)][int(0)][0u];
}

float16_t b(matrix<float16_t, 4, 2> m) {
  return m[int(0)][0u];
}

float16_t c(vector<float16_t, 2> v) {
  return v[0u];
}

float16_t d(float16_t f) {
  return f;
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_3.z) : (v_3.x)));
  uint4 v_5 = u[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.z) : (v_5.x)));
  uint4 v_7 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.z) : (v_7.x)));
  uint4 v_9 = u[((12u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 2>(v_4, v_6, v_8, tint_bitcast_to_f16(((((((12u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.z) : (v_9.x))));
}

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_10(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_2((start_byte_offset + (v_12 * 16u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_13[4] = a;
  return v_13;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> v_14[4] = v_10(0u);
  float16_t v_15 = a(v_14);
  float16_t v_16 = (v_15 + b(v_2(16u)));
  float16_t v_17 = (v_16 + c(tint_bitcast_to_f16(u[1u].x).yx));
  s.Store<float16_t>(0u, (v_17 + d(tint_bitcast_to_f16(u[1u].x).yx[0u])));
}

FXC validation failure:
<scrubbed_path>(6,1-9): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
