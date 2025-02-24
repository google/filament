struct Inner {
  matrix<float16_t, 2, 2> m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(a[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(a[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]));
}

Inner v_4(uint start_byte_offset) {
  Inner v_5 = {v_2(start_byte_offset)};
  return v_5;
}

typedef Inner ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      Inner v_9 = v_4((start_byte_offset + (v_8 * 64u)));
      a_2[v_8] = v_9;
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  Inner v_10[4] = a_2;
  return v_10;
}

Outer v_11(uint start_byte_offset) {
  Inner v_12[4] = v_6(start_byte_offset);
  Outer v_13 = {v_12};
  return v_13;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_14(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      Outer v_17 = v_11((start_byte_offset + (v_16 * 256u)));
      a_1[v_16] = v_17;
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  Outer v_18[4] = a_1;
  return v_18;
}

[numthreads(1, 1, 1)]
void f() {
  Outer l_a[4] = v_14(0u);
  Outer l_a_3 = v_11(768u);
  Inner l_a_3_a[4] = v_6(768u);
  Inner l_a_3_a_2 = v_4(896u);
  matrix<float16_t, 2, 2> l_a_3_a_2_m = v_2(896u);
  vector<float16_t, 2> l_a_3_a_2_m_1 = tint_bitcast_to_f16(a[56u].y);
  float16_t l_a_3_a_2_m_1_0 = float16_t(f16tof32(a[56u].y));
}

