
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
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

void v_4(uint offset, matrix<float16_t, 4, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
  s.Store<vector<float16_t, 3> >((offset + 24u), obj[3u]);
}

matrix<float16_t, 4, 3> v_5(uint start_byte_offset) {
  uint4 v_6 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy))).xyz;
  uint4 v_8 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_9 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy))).xyz;
  uint4 v_10 = u[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_11 = tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_10.zw) : (v_10.xy))).xyz;
  uint4 v_12 = u[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 3>(v_7, v_9, v_11, tint_bitcast_to_f16(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_12.zw) : (v_12.xy))).xyz);
}

void v_13(uint offset, matrix<float16_t, 4, 3> obj[4]) {
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      v_4((offset + (v_15 * 32u)), obj[v_15]);
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
}

typedef matrix<float16_t, 4, 3> ary_ret[4];
ary_ret v_16(uint start_byte_offset) {
  matrix<float16_t, 4, 3> a[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      a[v_18] = v_5((start_byte_offset + (v_18 * 32u)));
      {
        v_17 = (v_18 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 3> v_19[4] = a;
  return v_19;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 3> v_20[4] = v_16(0u);
  v_13(0u, v_20);
  v_4(32u, v_5(64u));
  s.Store<vector<float16_t, 3> >(32u, tint_bitcast_to_f16(u[0u].zw).xyz.zxy);
  s.Store<float16_t>(32u, float16_t(f16tof32(u[0u].z)));
}

