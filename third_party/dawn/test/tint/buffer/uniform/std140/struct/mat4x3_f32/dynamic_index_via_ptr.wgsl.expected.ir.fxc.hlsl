struct Inner {
  float4x3 m;
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

float4x3 v(uint start_byte_offset) {
  return float4x3(asfloat(a[(start_byte_offset / 16u)].xyz), asfloat(a[((16u + start_byte_offset) / 16u)].xyz), asfloat(a[((32u + start_byte_offset) / 16u)].xyz), asfloat(a[((48u + start_byte_offset) / 16u)].xyz));
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
  uint v_16 = (256u * min(uint(i()), 3u));
  uint v_17 = (64u * min(uint(i()), 3u));
  uint v_18 = (16u * min(uint(i()), 3u));
  Outer l_a[4] = v_11(0u);
  Outer l_a_i = v_8(v_16);
  Inner l_a_i_a[4] = v_3(v_16);
  Inner l_a_i_a_i = v_1((v_16 + v_17));
  float4x3 l_a_i_a_i_m = v((v_16 + v_17));
  float3 l_a_i_a_i_m_i = asfloat(a[(((v_16 + v_17) + v_18) / 16u)].xyz);
  uint v_19 = (((v_16 + v_17) + v_18) + (min(uint(i()), 2u) * 4u));
  float l_a_i_a_i_m_i_i = asfloat(a[(v_19 / 16u)][((v_19 % 16u) / 4u)]);
}

