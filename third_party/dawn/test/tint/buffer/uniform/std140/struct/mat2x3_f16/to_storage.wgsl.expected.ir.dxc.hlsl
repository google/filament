struct S {
  int before;
  matrix<float16_t, 2, 3> m;
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

void v_4(uint offset, matrix<float16_t, 2, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

matrix<float16_t, 2, 3> v_5(uint start_byte_offset) {
  uint4 v_6 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy))).xyz;
  uint4 v_8 = u[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_7, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy))).xyz);
}

void v_9(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_4((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_10(uint start_byte_offset) {
  int v_11 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 2, 3> v_12 = v_5((8u + start_byte_offset));
  S v_13 = {v_11, v_12, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_13;
}

void v_14(uint offset, S obj[4]) {
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      S v_17 = obj[v_16];
      v_9((offset + (v_16 * 128u)), v_17);
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
}

typedef S ary_ret[4];
ary_ret v_18(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      S v_21 = v_10((start_byte_offset + (v_20 * 128u)));
      a[v_20] = v_21;
      {
        v_19 = (v_20 + 1u);
      }
      continue;
    }
  }
  S v_22[4] = a;
  return v_22;
}

[numthreads(1, 1, 1)]
void f() {
  S v_23[4] = v_18(0u);
  v_14(0u, v_23);
  S v_24 = v_10(256u);
  v_9(128u, v_24);
  v_4(392u, v_5(264u));
  s.Store<vector<float16_t, 3> >(136u, tint_bitcast_to_f16(u[1u].xy).xyz.zxy);
}

