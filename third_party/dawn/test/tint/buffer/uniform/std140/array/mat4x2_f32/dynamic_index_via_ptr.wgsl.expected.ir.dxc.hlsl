
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

float4x2 v(uint start_byte_offset) {
  uint4 v_1 = a[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = a[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = a[((16u + start_byte_offset) / 16u)];
  float2 v_6 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = a[((24u + start_byte_offset) / 16u)];
  return float4x2(v_2, v_4, v_6, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

typedef float4x2 ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  float4x2 a_1[4] = (float4x2[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a_1[v_10] = v((start_byte_offset + (v_10 * 32u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  float4x2 v_11[4] = a_1;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_12 = (32u * min(uint(i()), 3u));
  uint v_13 = (8u * min(uint(i()), 3u));
  float4x2 l_a[4] = v_8(0u);
  float4x2 l_a_i = v(v_12);
  uint4 v_14 = a[((v_12 + v_13) / 16u)];
  float2 l_a_i_i = asfloat(((((((v_12 + v_13) % 16u) / 4u) == 2u)) ? (v_14.zw) : (v_14.xy)));
  s.Store(0u, asuint((((asfloat(a[((v_12 + v_13) / 16u)][(((v_12 + v_13) % 16u) / 4u)]) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x)));
}

