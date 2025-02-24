SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
float16_t a(matrix<float16_t, 4, 3> a_1[4]) {
  return a_1[int(0)][int(0)][0u];
}

float16_t b(matrix<float16_t, 4, 3> m) {
  return m[int(0)][0u];
}

float16_t c(vector<float16_t, 3> v) {
  return v[0u];
}

float16_t d(float16_t f) {
  return f;
}

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

matrix<float16_t, 4, 3> v_4(uint start_byte_offset) {
  vector<float16_t, 3> v_5 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]).xyz;
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]).xyz;
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16(u[((16u + start_byte_offset) / 16u)]).xyz;
  return matrix<float16_t, 4, 3>(v_5, v_6, v_7, tint_bitcast_to_f16(u[((24u + start_byte_offset) / 16u)]).xyz);
}

typedef matrix<float16_t, 4, 3> ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  matrix<float16_t, 4, 3> a[4] = (matrix<float16_t, 4, 3>[4])0;
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
  matrix<float16_t, 4, 3> v_11[4] = a;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 3> v_12[4] = v_8(0u);
  float16_t v_13 = a(v_12);
  float16_t v_14 = (v_13 + b(v_4(32u)));
  float16_t v_15 = (v_14 + c(tint_bitcast_to_f16(u[2u]).xyz.zxy));
  s.Store<float16_t>(0u, (v_15 + d(tint_bitcast_to_f16(u[2u]).xyz.zxy[0u])));
}

FXC validation failure:
<scrubbed_path>(6,1-9): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
