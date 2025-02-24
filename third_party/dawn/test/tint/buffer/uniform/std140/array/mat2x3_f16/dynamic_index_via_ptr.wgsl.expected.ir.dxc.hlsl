
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
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

matrix<float16_t, 2, 3> v_4(uint start_byte_offset) {
  uint4 v_5 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))).xyz;
  uint4 v_7 = a[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_6, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))).xyz);
}

typedef matrix<float16_t, 2, 3> ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  matrix<float16_t, 2, 3> a_1[4] = (matrix<float16_t, 2, 3>[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a_1[v_10] = v_4((start_byte_offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 2, 3> v_11[4] = a_1;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_12 = (16u * min(uint(i()), 3u));
  uint v_13 = (8u * min(uint(i()), 1u));
  matrix<float16_t, 2, 3> l_a[4] = v_8(0u);
  matrix<float16_t, 2, 3> l_a_i = v_4(v_12);
  uint4 v_14 = a[((v_12 + v_13) / 16u)];
  vector<float16_t, 3> l_a_i_i = tint_bitcast_to_f16(((((((v_12 + v_13) % 16u) / 4u) == 2u)) ? (v_14.zw) : (v_14.xy))).xyz;
  uint v_15 = a[((v_12 + v_13) / 16u)][(((v_12 + v_13) % 16u) / 4u)];
  s.Store<float16_t>(0u, (((float16_t(f16tof32((v_15 >> (((((v_12 + v_13) % 4u) == 0u)) ? (0u) : (16u))))) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x));
}

