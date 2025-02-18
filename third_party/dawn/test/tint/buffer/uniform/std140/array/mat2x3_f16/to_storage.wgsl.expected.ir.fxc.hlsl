SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
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

void v_4(uint offset, matrix<float16_t, 2, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

matrix<float16_t, 2, 3> v_5(uint start_byte_offset) {
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]).xyz;
  return matrix<float16_t, 2, 3>(v_6, tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]).xyz);
}

void v_7(uint offset, matrix<float16_t, 2, 3> obj[4]) {
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      v_4((offset + (v_9 * 16u)), obj[v_9]);
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
}

typedef matrix<float16_t, 2, 3> ary_ret[4];
ary_ret v_10(uint start_byte_offset) {
  matrix<float16_t, 2, 3> a[4] = (matrix<float16_t, 2, 3>[4])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_5((start_byte_offset + (v_12 * 16u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 2, 3> v_13[4] = a;
  return v_13;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 3> v_14[4] = v_10(0u);
  v_7(0u, v_14);
  v_4(16u, v_5(32u));
  s.Store<vector<float16_t, 3> >(16u, tint_bitcast_to_f16(u[0u]).xyz.zxy);
  s.Store<float16_t>(16u, float16_t(f16tof32(u[0u].z)));
}

FXC validation failure:
<scrubbed_path>(6,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
