
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
static float2x4 p[4] = (float2x4[4])0;
float2x4 v(uint start_byte_offset) {
  return float2x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]));
}

typedef float2x4 ary_ret[4];
ary_ret v_1(uint start_byte_offset) {
  float2x4 a[4] = (float2x4[4])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      a[v_3] = v((start_byte_offset + (v_3 * 32u)));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float2x4 v_4[4] = a;
  return v_4;
}

[numthreads(1, 1, 1)]
void f() {
  float2x4 v_5[4] = v_1(0u);
  p = v_5;
  p[1u] = v(64u);
  p[1u][0u] = asfloat(u[1u]).ywxz;
  p[1u][0u].x = asfloat(u[1u].x);
  s.Store(0u, asuint(p[1u][0u].x));
}

