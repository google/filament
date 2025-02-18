
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);
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

matrix<float16_t, 4, 4> v_4(uint start_byte_offset) {
  uint4 v_5 = a[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = a[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint4 v_9 = a[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_10 = tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy)));
  uint4 v_11 = a[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 4>(v_6, v_8, v_10, tint_bitcast_to_f16(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_11.zw) : (v_11.xy))));
}

typedef matrix<float16_t, 4, 4> ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  matrix<float16_t, 4, 4> a_1[4] = (matrix<float16_t, 4, 4>[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a_1[v_14] = v_4((start_byte_offset + (v_14 * 32u)));
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 4> v_15[4] = a_1;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 4> l_a[4] = v_12(0u);
  matrix<float16_t, 4, 4> l_a_i = v_4(64u);
  vector<float16_t, 4> l_a_i_i = tint_bitcast_to_f16(a[4u].zw);
  s.Store<float16_t>(0u, (((float16_t(f16tof32(a[4u].z)) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x));
}

