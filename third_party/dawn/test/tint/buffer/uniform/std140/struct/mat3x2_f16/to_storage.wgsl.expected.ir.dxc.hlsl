struct S {
  int before;
  matrix<float16_t, 3, 2> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

void v_2(uint offset, matrix<float16_t, 3, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

matrix<float16_t, 3, 2> v_3(uint start_byte_offset) {
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 3, 2>(v_4, v_5, tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]));
}

void v_6(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_2((offset + 4u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_7(uint start_byte_offset) {
  int v_8 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 3, 2> v_9 = v_3((4u + start_byte_offset));
  S v_10 = {v_8, v_9, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_10;
}

void v_11(uint offset, S obj[4]) {
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      S v_14 = obj[v_13];
      v_6((offset + (v_13 * 128u)), v_14);
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
}

typedef S ary_ret[4];
ary_ret v_15(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_16 = 0u;
    v_16 = 0u;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      S v_18 = v_7((start_byte_offset + (v_17 * 128u)));
      a[v_17] = v_18;
      {
        v_16 = (v_17 + 1u);
      }
      continue;
    }
  }
  S v_19[4] = a;
  return v_19;
}

[numthreads(1, 1, 1)]
void f() {
  S v_20[4] = v_15(0u);
  v_11(0u, v_20);
  S v_21 = v_7(256u);
  v_6(128u, v_21);
  v_2(388u, v_3(260u));
  s.Store<vector<float16_t, 2> >(132u, tint_bitcast_to_f16(u[0u].z).yx);
}

