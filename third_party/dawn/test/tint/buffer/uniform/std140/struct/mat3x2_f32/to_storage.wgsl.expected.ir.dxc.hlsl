struct S {
  int before;
  float3x2 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float3x2 obj) {
  s.Store2((offset + 0u), asuint(obj[0u]));
  s.Store2((offset + 8u), asuint(obj[1u]));
  s.Store2((offset + 16u), asuint(obj[2u]));
}

float3x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint4 v_4 = u[((8u + start_byte_offset) / 16u)];
  float2 v_5 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint4 v_6 = u[((16u + start_byte_offset) / 16u)];
  return float3x2(v_3, v_5, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy))));
}

void v_7(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_8(uint start_byte_offset) {
  int v_9 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float3x2 v_10 = v_1((8u + start_byte_offset));
  S v_11 = {v_9, v_10, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_11;
}

void v_12(uint offset, S obj[4]) {
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      S v_15 = obj[v_14];
      v_7((offset + (v_14 * 128u)), v_15);
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
}

typedef S ary_ret[4];
ary_ret v_16(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      S v_19 = v_8((start_byte_offset + (v_18 * 128u)));
      a[v_18] = v_19;
      {
        v_17 = (v_18 + 1u);
      }
      continue;
    }
  }
  S v_20[4] = a;
  return v_20;
}

[numthreads(1, 1, 1)]
void f() {
  S v_21[4] = v_16(0u);
  v_12(0u, v_21);
  S v_22 = v_8(256u);
  v_7(128u, v_22);
  v(392u, v_1(264u));
  s.Store2(136u, asuint(asfloat(u[1u].xy).yx));
}

