SKIP: INVALID

struct Inner {
  matrix<float16_t, 4, 2> m;
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

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  uint4 v_3 = a[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_3.z) : (v_3.x)));
  uint4 v_5 = a[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.z) : (v_5.x)));
  uint4 v_7 = a[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.z) : (v_7.x)));
  uint4 v_9 = a[((12u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 2>(v_4, v_6, v_8, tint_bitcast_to_f16(((((((12u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.z) : (v_9.x))));
}

Inner v_10(uint start_byte_offset) {
  Inner v_11 = {v_2(start_byte_offset)};
  return v_11;
}

typedef Inner ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      Inner v_15 = v_10((start_byte_offset + (v_14 * 64u)));
      a[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  Inner v_16[4] = a;
  return v_16;
}

Outer v_17(uint start_byte_offset) {
  Inner v_18[4] = v_12(start_byte_offset);
  Outer v_19 = {v_18};
  return v_19;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_20(uint start_byte_offset) {
  Outer a[4] = (Outer[4])0;
  {
    uint v_21 = 0u;
    v_21 = 0u;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 4u)) {
        break;
      }
      Outer v_23 = v_17((start_byte_offset + (v_22 * 256u)));
      a[v_22] = v_23;
      {
        v_21 = (v_22 + 1u);
      }
      continue;
    }
  }
  Outer v_24[4] = a;
  return v_24;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_25 = (256u * uint(i()));
  uint v_26 = (64u * uint(i()));
  uint v_27 = (4u * uint(i()));
  Outer l_a[4] = v_20(0u);
  Outer l_a_i = v_17(v_25);
  Inner l_a_i_a[4] = v_12(v_25);
  Inner l_a_i_a_i = v_10((v_25 + v_26));
  matrix<float16_t, 4, 2> l_a_i_a_i_m = v_2((v_25 + v_26));
  uint4 v_28 = a[(((v_25 + v_26) + v_27) / 16u)];
  vector<float16_t, 2> l_a_i_a_i_m_i = tint_bitcast_to_f16((((((((v_25 + v_26) + v_27) % 16u) / 4u) == 2u)) ? (v_28.z) : (v_28.x)));
  uint v_29 = (((v_25 + v_26) + v_27) + (uint(i()) * 2u));
  uint v_30 = a[(v_29 / 16u)][((v_29 % 16u) / 4u)];
  float16_t l_a_i_a_i_m_i_i = float16_t(f16tof32((v_30 >> ((((v_29 % 4u) == 0u)) ? (0u) : (16u)))));
}

FXC validation failure:
<scrubbed_path>(2,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
