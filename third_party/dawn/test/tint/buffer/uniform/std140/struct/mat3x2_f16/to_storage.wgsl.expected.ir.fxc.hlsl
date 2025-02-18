SKIP: INVALID

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
  uint4 v_4 = u[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_4.z) : (v_4.x)));
  uint4 v_6 = u[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_6.z) : (v_6.x)));
  uint4 v_8 = u[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 2>(v_5, v_7, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.z) : (v_8.x))));
}

void v_9(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_2((offset + 4u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_10(uint start_byte_offset) {
  int v_11 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 3, 2> v_12 = v_3((4u + start_byte_offset));
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
  v_2(388u, v_3(260u));
  s.Store<vector<float16_t, 2> >(132u, tint_bitcast_to_f16(u[0u].z).yx);
}

FXC validation failure:
<scrubbed_path>(3,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
