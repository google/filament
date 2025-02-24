SKIP: INVALID

struct S {
  int before;
  matrix<float16_t, 3, 3> m;
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

void v_4(uint offset, matrix<float16_t, 3, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
}

matrix<float16_t, 3, 3> v_5(uint start_byte_offset) {
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]).xyz;
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]).xyz;
  return matrix<float16_t, 3, 3>(v_6, v_7, tint_bitcast_to_f16(u[((16u + start_byte_offset) / 16u)]).xyz);
}

void v_8(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_4((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_9(uint start_byte_offset) {
  int v_10 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 3, 3> v_11 = v_5((8u + start_byte_offset));
  S v_12 = {v_10, v_11, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_12;
}

void v_13(uint offset, S obj[4]) {
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      S v_16 = obj[v_15];
      v_8((offset + (v_15 * 128u)), v_16);
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
}

typedef S ary_ret[4];
ary_ret v_17(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      S v_20 = v_9((start_byte_offset + (v_19 * 128u)));
      a[v_19] = v_20;
      {
        v_18 = (v_19 + 1u);
      }
      continue;
    }
  }
  S v_21[4] = a;
  return v_21;
}

[numthreads(1, 1, 1)]
void f() {
  S v_22[4] = v_17(0u);
  v_13(0u, v_22);
  S v_23 = v_9(256u);
  v_8(128u, v_23);
  v_4(392u, v_5(264u));
  s.Store<vector<float16_t, 3> >(136u, tint_bitcast_to_f16(u[1u]).xyz.zxy);
}

FXC validation failure:
<scrubbed_path>(3,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
