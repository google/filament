
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

void v_2(uint offset, matrix<float16_t, 4, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  s.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

matrix<float16_t, 4, 2> v_3(uint start_byte_offset) {
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 4, 2>(v_4, v_5, v_6, tint_bitcast_to_f16(u[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]));
}

void v_7(uint offset, matrix<float16_t, 4, 2> obj[4]) {
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      v_2((offset + (v_9 * 16u)), obj[v_9]);
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
}

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_10(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_3((start_byte_offset + (v_12 * 16u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_13[4] = a;
  return v_13;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 2> v_14[4] = v_10(0u);
  v_7(0u, v_14);
  v_2(16u, v_3(32u));
  s.Store<vector<float16_t, 2> >(16u, tint_bitcast_to_f16(u[0u].y).yx);
  s.Store<float16_t>(16u, float16_t(f16tof32(u[0u].y)));
}

