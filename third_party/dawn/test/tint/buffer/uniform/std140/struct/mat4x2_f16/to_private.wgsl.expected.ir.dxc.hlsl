struct S {
  int before;
  matrix<float16_t, 4, 2> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 4, 2>(v_3, v_4, v_5, tint_bitcast_to_f16(u[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]));
}

S v_6(uint start_byte_offset) {
  int v_7 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 4, 2> v_8 = v_2((4u + start_byte_offset));
  S v_9 = {v_7, v_8, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_9;
}

typedef S ary_ret[4];
ary_ret v_10(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      S v_13 = v_6((start_byte_offset + (v_12 * 128u)));
      a[v_12] = v_13;
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  S v_14[4] = a;
  return v_14;
}

[numthreads(1, 1, 1)]
void f() {
  S v_15[4] = v_10(0u);
  p = v_15;
  S v_16 = v_6(256u);
  p[1u] = v_16;
  p[3u].m = v_2(260u);
  p[1u].m[0u] = tint_bitcast_to_f16(u[0u].z).yx;
}

