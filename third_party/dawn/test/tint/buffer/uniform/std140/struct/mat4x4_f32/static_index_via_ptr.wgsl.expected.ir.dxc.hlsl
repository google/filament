struct Inner {
  float4x4 m;
};

struct Outer {
  Inner a[4];
};


cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
float4x4 v(uint start_byte_offset) {
  return float4x4(asfloat(a[(start_byte_offset / 16u)]), asfloat(a[((16u + start_byte_offset) / 16u)]), asfloat(a[((32u + start_byte_offset) / 16u)]), asfloat(a[((48u + start_byte_offset) / 16u)]));
}

Inner v_1(uint start_byte_offset) {
  Inner v_2 = {v(start_byte_offset)};
  return v_2;
}

typedef Inner ary_ret[4];
ary_ret v_3(uint start_byte_offset) {
  Inner a_2[4] = (Inner[4])0;
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      Inner v_6 = v_1((start_byte_offset + (v_5 * 64u)));
      a_2[v_5] = v_6;
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  Inner v_7[4] = a_2;
  return v_7;
}

Outer v_8(uint start_byte_offset) {
  Inner v_9[4] = v_3(start_byte_offset);
  Outer v_10 = {v_9};
  return v_10;
}

typedef Outer ary_ret_1[4];
ary_ret_1 v_11(uint start_byte_offset) {
  Outer a_1[4] = (Outer[4])0;
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      Outer v_14 = v_8((start_byte_offset + (v_13 * 256u)));
      a_1[v_13] = v_14;
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
  Outer v_15[4] = a_1;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  Outer l_a[4] = v_11(0u);
  Outer l_a_3 = v_8(768u);
  Inner l_a_3_a[4] = v_3(768u);
  Inner l_a_3_a_2 = v_1(896u);
  float4x4 l_a_3_a_2_m = v(896u);
  float4 l_a_3_a_2_m_1 = asfloat(a[57u]);
  float l_a_3_a_2_m_1_0 = asfloat(a[57u].x);
}

