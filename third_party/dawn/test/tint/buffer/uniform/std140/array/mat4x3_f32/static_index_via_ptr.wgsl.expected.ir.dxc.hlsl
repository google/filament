
cbuffer cbuffer_a : register(b0) {
  uint4 a[16];
};
RWByteAddressBuffer s : register(u1);
float4x3 v(uint start_byte_offset) {
  return float4x3(asfloat(a[(start_byte_offset / 16u)].xyz), asfloat(a[((16u + start_byte_offset) / 16u)].xyz), asfloat(a[((32u + start_byte_offset) / 16u)].xyz), asfloat(a[((48u + start_byte_offset) / 16u)].xyz));
}

typedef float4x3 ary_ret[4];
ary_ret v_1(uint start_byte_offset) {
  float4x3 a_1[4] = (float4x3[4])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      a_1[v_3] = v((start_byte_offset + (v_3 * 64u)));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float4x3 v_4[4] = a_1;
  return v_4;
}

[numthreads(1, 1, 1)]
void f() {
  float4x3 l_a[4] = v_1(0u);
  float4x3 l_a_i = v(128u);
  float3 l_a_i_i = asfloat(a[9u].xyz);
  s.Store(0u, asuint((((asfloat(a[9u].x) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x)));
}

