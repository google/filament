
cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);
float2x3 v(uint start_byte_offset) {
  return float2x3(asfloat(a[(start_byte_offset / 16u)].xyz), asfloat(a[((16u + start_byte_offset) / 16u)].xyz));
}

typedef float2x3 ary_ret[4];
ary_ret v_1(uint start_byte_offset) {
  float2x3 a_1[4] = (float2x3[4])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      a_1[v_3] = v((start_byte_offset + (v_3 * 32u)));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float2x3 v_4[4] = a_1;
  return v_4;
}

[numthreads(1, 1, 1)]
void f() {
  float2x3 l_a[4] = v_1(0u);
  float2x3 l_a_i = v(64u);
  float3 l_a_i_i = asfloat(a[5u].xyz);
  s.Store(0u, asuint((((asfloat(a[5u].x) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x)));
}

