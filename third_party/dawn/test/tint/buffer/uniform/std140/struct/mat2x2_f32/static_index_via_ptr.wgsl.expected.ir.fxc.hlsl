struct Inner {
  float2x2 m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = a[((8u + start_byte_offset) / 16u)];
  return float2x2(v_2, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy))));
}

Inner v_4(uint start_byte_offset) {
  Inner v_5 = {v(start_byte_offset)};
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
  float2x2 l_a_3_a_2_m = v(896u);
  float2 l_a_3_a_2_m_1 = asfloat(a[56u].zw);
  float l_a_3_a_2_m_1_0 = asfloat(a[56u].z);
}

