SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
static matrix<float16_t, 2, 3> p[4] = (matrix<float16_t, 2, 3>[4])0;
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

matrix<float16_t, 2, 3> v_4(uint start_byte_offset) {
  vector<float16_t, 3> v_5 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]).xyz;
  return matrix<float16_t, 2, 3>(v_5, tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]).xyz);
}

typedef matrix<float16_t, 2, 3> ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  matrix<float16_t, 2, 3> a[4] = (matrix<float16_t, 2, 3>[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_4((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 2, 3> v_9[4] = a;
  return v_9;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 3> v_10[4] = v_6(0u);
  p = v_10;
  p[int(1)] = v_4(32u);
  p[int(1)][int(0)] = tint_bitcast_to_f16(u[0u]).xyz.zxy;
  p[int(1)][int(0)][0u] = float16_t(f16tof32(u[0u].z));
  s.Store<float16_t>(0u, p[int(1)][int(0)].x);
}

FXC validation failure:
<scrubbed_path>(6,15-23): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
