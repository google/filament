struct Inner {
  matrix<float16_t, 2, 2> m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
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
  uint v_19 = (256u * min(uint(i()), 3u));
  uint v_20 = (64u * min(uint(i()), 3u));
  uint v_21 = (4u * min(uint(i()), 1u));
  Outer l_a[4] = v_14(0u);
  Outer l_a_i = v_11(v_19);
  Inner l_a_i_a[4] = v_6(v_19);
  Inner l_a_i_a_i = v_4((v_19 + v_20));
  matrix<float16_t, 2, 2> l_a_i_a_i_m = v_2((v_19 + v_20));
  vector<float16_t, 2> l_a_i_a_i_m_i = tint_bitcast_to_f16(a[(((v_19 + v_20) + v_21) / 16u)][((((v_19 + v_20) + v_21) % 16u) / 4u)]);
  uint v_22 = (((v_19 + v_20) + v_21) + (min(uint(i()), 1u) * 2u));
  uint v_23 = a[(v_22 / 16u)][((v_22 % 16u) / 4u)];
  float16_t l_a_i_a_i_m_i_i = float16_t(f16tof32((v_23 >> ((((v_22 % 4u) == 0u)) ? (0u) : (16u)))));
}

