SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
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

void v_4(uint offset, matrix<float16_t, 4, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
  s.Store<vector<float16_t, 3> >((offset + 24u), obj[3u]);
}

matrix<float16_t, 4, 3> v_5(uint start_byte_offset) {
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]).xyz;
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]).xyz;
  vector<float16_t, 3> v_8 = tint_bitcast_to_f16(u[((16u + start_byte_offset) / 16u)]).xyz;
  return matrix<float16_t, 4, 3>(v_6, v_7, v_8, tint_bitcast_to_f16(u[((24u + start_byte_offset) / 16u)]).xyz);
}

void v_9(uint offset, matrix<float16_t, 4, 3> obj[4]) {
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      v_4((offset + (v_11 * 32u)), obj[v_11]);
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
}

typedef matrix<float16_t, 4, 3> ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  matrix<float16_t, 4, 3> a[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a[v_14] = v_5((start_byte_offset + (v_14 * 32u)));
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 3> v_15[4] = a;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 3> v_16[4] = v_12(0u);
  v_9(0u, v_16);
  v_4(32u, v_5(64u));
  s.Store<vector<float16_t, 3> >(32u, tint_bitcast_to_f16(u[0u]).xyz.zxy);
  s.Store<float16_t>(32u, float16_t(f16tof32(u[0u].z)));
}

FXC validation failure:
<scrubbed_path>(6,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
