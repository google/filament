
cbuffer cbuffer_a : register(b0) {
  uint4 a[16];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

float4x4 v(uint start_byte_offset) {
  return float4x4(asfloat(a[(start_byte_offset / 16u)]), asfloat(a[((16u + start_byte_offset) / 16u)]), asfloat(a[((32u + start_byte_offset) / 16u)]), asfloat(a[((48u + start_byte_offset) / 16u)]));
}

typedef float4x4 ary_ret[4];
ary_ret v_1(uint start_byte_offset) {
  float4x4 a_1[4] = (float4x4[4])0;
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
  float4x4 v_4[4] = a_1;
  return v_4;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_5 = (64u * min(uint(i()), 3u));
  uint v_6 = (16u * min(uint(i()), 3u));
  float4x4 l_a[4] = v_1(0u);
  float4x4 l_a_i = v(v_5);
  float4 l_a_i_i = asfloat(a[((v_5 + v_6) / 16u)]);
  s.Store(0u, asuint((((asfloat(a[((v_5 + v_6) / 16u)][(((v_5 + v_6) % 16u) / 4u)]) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x)));
}

