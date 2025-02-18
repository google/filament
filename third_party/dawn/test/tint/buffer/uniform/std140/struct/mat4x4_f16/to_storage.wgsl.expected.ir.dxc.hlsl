struct S {
  int before;
  matrix<float16_t, 4, 4> m;
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

void v_4(uint offset, matrix<float16_t, 4, 4> obj) {
  s.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  s.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

matrix<float16_t, 4, 4> v_5(uint start_byte_offset) {
  uint4 v_6 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy)));
  uint4 v_8 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_9 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy)));
  uint4 v_10 = u[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_11 = tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_10.zw) : (v_10.xy)));
  uint4 v_12 = u[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 4>(v_7, v_9, v_11, tint_bitcast_to_f16(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_12.zw) : (v_12.xy))));
}

void v_13(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_4((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_14(uint start_byte_offset) {
  int v_15 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 4, 4> v_16 = v_5((8u + start_byte_offset));
  S v_17 = {v_15, v_16, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_17;
}

void v_18(uint offset, S obj[4]) {
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      S v_21 = obj[v_20];
      v_13((offset + (v_20 * 128u)), v_21);
      {
        v_19 = (v_20 + 1u);
      }
      continue;
    }
  }
}

typedef S ary_ret[4];
ary_ret v_22(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_23 = 0u;
    v_23 = 0u;
    while(true) {
      uint v_24 = v_23;
      if ((v_24 >= 4u)) {
        break;
      }
      S v_25 = v_14((start_byte_offset + (v_24 * 128u)));
      a[v_24] = v_25;
      {
        v_23 = (v_24 + 1u);
      }
      continue;
    }
  }
  S v_26[4] = a;
  return v_26;
}

[numthreads(1, 1, 1)]
void f() {
  S v_27[4] = v_22(0u);
  v_18(0u, v_27);
  S v_28 = v_14(256u);
  v_13(128u, v_28);
  v_4(392u, v_5(264u));
  s.Store<vector<float16_t, 4> >(136u, tint_bitcast_to_f16(u[1u].xy).ywxz);
}

