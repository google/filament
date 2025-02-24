struct S {
  int before;
  matrix<float16_t, 2, 2> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(matrix<float16_t, 2, 2> m) {
}

void d(vector<float16_t, 2> v) {
}

void e(float16_t f_1) {
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(u[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]));
}

S v_4(uint start_byte_offset) {
  int v_5 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 2, 2> v_6 = v_2((4u + start_byte_offset));
  S v_7 = {v_5, v_6, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_7;
}

typedef S ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      S v_11 = v_4((start_byte_offset + (v_10 * 128u)));
      a_2[v_10] = v_11;
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  S v_12[4] = a_2;
  return v_12;
}

[numthreads(1, 1, 1)]
void f() {
  S v_13[4] = v_8(0u);
  a(v_13);
  S v_14 = v_4(256u);
  b(v_14);
  c(v_2(260u));
  d(tint_bitcast_to_f16(u[0u].z).yx);
  e(tint_bitcast_to_f16(u[0u].z).yx.x);
}

