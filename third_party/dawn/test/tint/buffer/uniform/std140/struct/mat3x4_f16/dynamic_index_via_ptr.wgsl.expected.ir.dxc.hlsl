struct Inner {
  matrix<float16_t, 3, 4> m;
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

matrix<float16_t, 3, 4> v_4(uint start_byte_offset) {
  uint4 v_5 = a[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = a[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint4 v_9 = a[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 4>(v_6, v_8, tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy))));
}

Inner v_10(uint start_byte_offset) {
  Inner v_11 = {v_4(start_byte_offset)};
  return v_11;
}

typedef Inner ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      Inner v_15 = v_10((start_byte_offset + (v_14 * 64u)));
      a_2[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  Inner v_16[4] = a_2;
  return v_16;
}

Outer v_17(uint start_byte_offset) {
  Inner v_18[4] = v_12(start_byte_offset);
  Outer v_19 = {v_18};
  return v_19;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_20(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_21 = 0u;
    v_21 = 0u;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 4u)) {
        break;
      }
      Outer v_23 = v_17((start_byte_offset + (v_22 * 256u)));
      a_1[v_22] = v_23;
      {
        v_21 = (v_22 + 1u);
      }
      continue;
    }
  }
  Outer v_24[4] = a_1;
  return v_24;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_25 = (256u * min(uint(i()), 3u));
  uint v_26 = (64u * min(uint(i()), 3u));
  uint v_27 = (8u * min(uint(i()), 2u));
  Outer l_a[4] = v_20(0u);
  Outer l_a_i = v_17(v_25);
  Inner l_a_i_a[4] = v_12(v_25);
  Inner l_a_i_a_i = v_10((v_25 + v_26));
  matrix<float16_t, 3, 4> l_a_i_a_i_m = v_4((v_25 + v_26));
  uint4 v_28 = a[(((v_25 + v_26) + v_27) / 16u)];
  vector<float16_t, 4> l_a_i_a_i_m_i = tint_bitcast_to_f16((((((((v_25 + v_26) + v_27) % 16u) / 4u) == 2u)) ? (v_28.zw) : (v_28.xy)));
  uint v_29 = (((v_25 + v_26) + v_27) + (min(uint(i()), 3u) * 2u));
  uint v_30 = a[(v_29 / 16u)][((v_29 % 16u) / 4u)];
  float16_t l_a_i_a_i_m_i_i = float16_t(f16tof32((v_30 >> ((((v_29 % 4u) == 0u)) ? (0u) : (16u)))));
}

