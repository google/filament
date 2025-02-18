struct S {
  int before;
  float2x2 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
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

void v_5(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_6(uint start_byte_offset) {
  int v_7 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float2x2 v_8 = v_1((8u + start_byte_offset));
  S v_9 = {v_7, v_8, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_9;
}

void v_10(uint offset, S obj[4]) {
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      S v_13 = obj[v_12];
      v_5((offset + (v_12 * 128u)), v_13);
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
}

typedef S ary_ret[4];
ary_ret v_14(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      S v_17 = v_6((start_byte_offset + (v_16 * 128u)));
      a[v_16] = v_17;
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  S v_18[4] = a;
  return v_18;
}

[numthreads(1, 1, 1)]
void f() {
  S v_19[4] = v_14(0u);
  v_10(0u, v_19);
  S v_20 = v_6(256u);
  v_5(128u, v_20);
  v(392u, v_1(264u));
  s.Store2(136u, asuint(asfloat(u[1u].xy).yx));
}

