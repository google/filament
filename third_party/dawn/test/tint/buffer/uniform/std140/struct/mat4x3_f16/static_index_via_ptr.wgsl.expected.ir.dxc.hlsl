struct Inner {
  matrix<float16_t, 4, 3> m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
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

matrix<float16_t, 4, 3> v_4(uint start_byte_offset) {
  uint4 v_5 = a[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))).xyz;
  uint4 v_7 = a[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))).xyz;
  uint4 v_9 = a[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_10 = tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy))).xyz;
  uint4 v_11 = a[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 3>(v_6, v_8, v_10, tint_bitcast_to_f16(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_11.zw) : (v_11.xy))).xyz);
}

Inner v_12(uint start_byte_offset) {
  Inner v_13 = {v_4(start_byte_offset)};
  return v_13;
}

typedef Inner ary_ret[4];
ary_ret v_14(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      Inner v_17 = v_12((start_byte_offset + (v_16 * 64u)));
      a_2[v_16] = v_17;
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  Inner v_18[4] = a_2;
  return v_18;
}

Outer v_19(uint start_byte_offset) {
  Inner v_20[4] = v_14(start_byte_offset);
  Outer v_21 = {v_20};
  return v_21;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_22(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_23 = 0u;
    v_23 = 0u;
    while(true) {
      uint v_24 = v_23;
      if ((v_24 >= 4u)) {
        break;
      }
      Outer v_25 = v_19((start_byte_offset + (v_24 * 256u)));
      a_1[v_24] = v_25;
      {
        v_23 = (v_24 + 1u);
      }
      continue;
    }
  }
  Outer v_26[4] = a_1;
  return v_26;
}

[numthreads(1, 1, 1)]
void f() {
  Outer l_a[4] = v_22(0u);
  Outer l_a_3 = v_19(768u);
  Inner l_a_3_a[4] = v_14(768u);
  Inner l_a_3_a_2 = v_12(896u);
  matrix<float16_t, 4, 3> l_a_3_a_2_m = v_4(896u);
  vector<float16_t, 3> l_a_3_a_2_m_1 = tint_bitcast_to_f16(a[56u].zw).xyz;
  float16_t l_a_3_a_2_m_1_0 = float16_t(f16tof32(a[56u].z));
}

