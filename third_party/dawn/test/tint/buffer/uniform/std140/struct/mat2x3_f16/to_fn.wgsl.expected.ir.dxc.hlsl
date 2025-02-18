struct S {
  int before;
  matrix<float16_t, 2, 3> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(matrix<float16_t, 2, 3> m) {
}

void d(vector<float16_t, 3> v) {
}

void e(float16_t f_1) {
}

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

matrix<float16_t, 2, 3> v_4(uint start_byte_offset) {
  uint4 v_5 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))).xyz;
  uint4 v_7 = u[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_6, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))).xyz);
}

S v_8(uint start_byte_offset) {
  int v_9 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 2, 3> v_10 = v_4((8u + start_byte_offset));
  S v_11 = {v_9, v_10, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_11;
}

typedef S ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      S v_15 = v_8((start_byte_offset + (v_14 * 128u)));
      a_2[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  S v_16[4] = a_2;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  S v_17[4] = v_12(0u);
  a(v_17);
  S v_18 = v_8(256u);
  b(v_18);
  c(v_4(264u));
  d(tint_bitcast_to_f16(u[1u].xy).xyz.zxy);
  e(tint_bitcast_to_f16(u[1u].xy).xyz.zxy.x);
}

