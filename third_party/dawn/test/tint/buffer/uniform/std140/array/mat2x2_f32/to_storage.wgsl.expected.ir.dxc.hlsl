
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float2x2 obj) {
  s.Store2((offset + 0u), asuint(obj[0u]));
  s.Store2((offset + 8u), asuint(obj[1u]));
}

float2x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint4 v_4 = u[((8u + start_byte_offset) / 16u)];
  return float2x2(v_3, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

void v_5(uint offset, float2x2 obj[4]) {
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      v((offset + (v_7 * 16u)), obj[v_7]);
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
}

typedef float2x2 ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  float2x2 a[4] = (float2x2[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_1((start_byte_offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  float2x2 v_11[4] = a;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 v_12[4] = v_8(0u);
  v_5(0u, v_12);
  v(16u, v_1(32u));
  s.Store2(16u, asuint(asfloat(u[0u].zw).yx));
  s.Store(16u, asuint(asfloat(u[0u].z)));
}

