struct S {
  int before;
  float3x4 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;
float3x4 v(uint start_byte_offset) {
  return float3x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]), asfloat(u[((32u + start_byte_offset) / 16u)]));
}

S v_1(uint start_byte_offset) {
  int v_2 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float3x4 v_3 = v((16u + start_byte_offset));
  S v_4 = {v_2, v_3, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_4;
}

typedef S ary_ret[4];
ary_ret v_5(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      S v_8 = v_1((start_byte_offset + (v_7 * 128u)));
      a[v_7] = v_8;
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  S v_9[4] = a;
  return v_9;
}

[numthreads(1, 1, 1)]
void f() {
  S v_10[4] = v_5(0u);
  p = v_10;
  S v_11 = v_1(256u);
  p[1u] = v_11;
  p[3u].m = v(272u);
  p[1u].m[0u] = asfloat(u[2u]).ywxz;
}

