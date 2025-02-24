SKIP: INVALID

struct Inner {
  matrix<float16_t, 2, 4> m;
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

vector<float16_t, 4> tint_bitcast_to_f16(uint4 src) {
  uint4 v = src;
  uint4 mask = (65535u).xxxx;
  uint4 shift = (16u).xxxx;
  float4 t_low = f16tof32((v & mask));
  float4 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

matrix<float16_t, 2, 4> v_4(uint start_byte_offset) {
  vector<float16_t, 4> v_5 = tint_bitcast_to_f16(a[(start_byte_offset / 16u)]);
  return matrix<float16_t, 2, 4>(v_5, tint_bitcast_to_f16(a[((8u + start_byte_offset) / 16u)]));
}

Inner v_6(uint start_byte_offset) {
  Inner v_7 = {v_4(start_byte_offset)};
  return v_7;
}

typedef Inner ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      Inner v_11 = v_6((start_byte_offset + (v_10 * 64u)));
      a[v_10] = v_11;
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  Inner v_12[4] = a;
  return v_12;
}

Outer v_13(uint start_byte_offset) {
  Inner v_14[4] = v_8(start_byte_offset);
  Outer v_15 = {v_14};
  return v_15;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_16(uint start_byte_offset) {
  Outer a[4] = (Outer[4])0;
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      Outer v_19 = v_13((start_byte_offset + (v_18 * 256u)));
      a[v_18] = v_19;
      {
        v_17 = (v_18 + 1u);
      }
      continue;
    }
  }
  Outer v_20[4] = a;
  return v_20;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_21 = (256u * uint(i()));
  uint v_22 = (64u * uint(i()));
  uint v_23 = (8u * uint(i()));
  Outer l_a[4] = v_16(0u);
  Outer l_a_i = v_13(v_21);
  Inner l_a_i_a[4] = v_8(v_21);
  Inner l_a_i_a_i = v_6((v_21 + v_22));
  matrix<float16_t, 2, 4> l_a_i_a_i_m = v_4((v_21 + v_22));
  vector<float16_t, 4> l_a_i_a_i_m_i = tint_bitcast_to_f16(a[(((v_21 + v_22) + v_23) / 16u)]);
  uint v_24 = (((v_21 + v_22) + v_23) + (uint(i()) * 2u));
  uint v_25 = a[(v_24 / 16u)][((v_24 % 16u) / 4u)];
  float16_t l_a_i_a_i_m_i_i = float16_t(f16tof32((v_25 >> ((((v_24 % 4u) == 0u)) ? (0u) : (16u)))));
}

FXC validation failure:
<scrubbed_path>(2,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
