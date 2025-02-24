SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
static matrix<float16_t, 4, 4> p[4] = (matrix<float16_t, 4, 4>[4])0;
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

matrix<float16_t, 4, 4> v_4(uint start_byte_offset) {
  vector<float16_t, 4> v_5 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]);
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]);
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16(u[((16u + start_byte_offset) / 16u)]);
  return matrix<float16_t, 4, 4>(v_5, v_6, v_7, tint_bitcast_to_f16(u[((24u + start_byte_offset) / 16u)]));
}

typedef matrix<float16_t, 4, 4> ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  matrix<float16_t, 4, 4> a[4] = (matrix<float16_t, 4, 4>[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_4((start_byte_offset + (v_10 * 32u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 4> v_11[4] = a;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 4> v_12[4] = v_8(0u);
  p = v_12;
  p[int(1)] = v_4(64u);
  p[int(1)][int(0)] = tint_bitcast_to_f16(u[0u]).ywxz;
  p[int(1)][int(0)][0u] = float16_t(f16tof32(u[0u].z));
  s.Store<float16_t>(0u, p[int(1)][int(0)].x);
}

FXC validation failure:
<scrubbed_path>(6,15-23): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
