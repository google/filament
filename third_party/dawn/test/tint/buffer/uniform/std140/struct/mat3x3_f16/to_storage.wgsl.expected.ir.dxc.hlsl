struct S {
  int before;
  matrix<float16_t, 3, 3> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

void v_4(uint offset, matrix<float16_t, 3, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
}

matrix<float16_t, 3, 3> v_5(uint start_byte_offset) {
  uint4 v_6 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy))).xyz;
  uint4 v_8 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_9 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy))).xyz;
  uint4 v_10 = u[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 3>(v_7, v_9, tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_10.zw) : (v_10.xy))).xyz);
}

void v_11(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_4((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_12(uint start_byte_offset) {
  int v_13 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 3, 3> v_14 = v_5((8u + start_byte_offset));
  S v_15 = {v_13, v_14, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_15;
}

void v_16(uint offset, S obj[4]) {
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      S v_19 = obj[v_18];
      v_11((offset + (v_18 * 128u)), v_19);
      {
        v_17 = (v_18 + 1u);
      }
      continue;
    }
  }
}

typedef S ary_ret[4];
ary_ret v_20(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_21 = 0u;
    v_21 = 0u;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 4u)) {
        break;
      }
      S v_23 = v_12((start_byte_offset + (v_22 * 128u)));
      a[v_22] = v_23;
      {
        v_21 = (v_22 + 1u);
      }
      continue;
    }
  }
  S v_24[4] = a;
  return v_24;
}

[numthreads(1, 1, 1)]
void f() {
  S v_25[4] = v_20(0u);
  v_16(0u, v_25);
  S v_26 = v_12(256u);
  v_11(128u, v_26);
  v_4(392u, v_5(264u));
  s.Store<vector<float16_t, 3> >(136u, tint_bitcast_to_f16(u[1u].xy).xyz.zxy);
}

