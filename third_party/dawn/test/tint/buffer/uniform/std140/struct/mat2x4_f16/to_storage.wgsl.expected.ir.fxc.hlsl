SKIP: INVALID

struct S {
  int before;
  matrix<float16_t, 2, 4> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 4> tint_bitcast_to_f16(uint4 src) {
  uint4 v = src;
  uint4 mask = (65535u).xxxx;
  uint4 shift = (16u).xxxx;
  float4 t_low = f16tof32((v & mask));
  float4 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

void v_4(uint offset, matrix<float16_t, 2, 4> obj) {
  s.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

matrix<float16_t, 2, 4> v_5(uint start_byte_offset) {
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]);
  return matrix<float16_t, 2, 4>(v_6, tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]));
}

void v_7(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_4((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_8(uint start_byte_offset) {
  int v_9 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 2, 4> v_10 = v_5((8u + start_byte_offset));
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
  v_4(392u, v_5(264u));
  s.Store<vector<float16_t, 4> >(136u, tint_bitcast_to_f16(u[1u]).ywxz);
}

FXC validation failure:
<scrubbed_path>(3,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
