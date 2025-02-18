SKIP: INVALID

struct S {
  int before;
  matrix<float16_t, 4, 2> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(matrix<float16_t, 4, 2> m) {
}

void d(vector<float16_t, 2> v) {
}

void e(float16_t f) {
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_3.z) : (v_3.x)));
  uint4 v_5 = u[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.z) : (v_5.x)));
  uint4 v_7 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.z) : (v_7.x)));
  uint4 v_9 = u[((12u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 2>(v_4, v_6, v_8, tint_bitcast_to_f16(((((((12u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.z) : (v_9.x))));
}

S v_10(uint start_byte_offset) {
  int v_11 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 4, 2> v_12 = v_2((4u + start_byte_offset));
  S v_13 = {v_11, v_12, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_13;
}

typedef S ary_ret[4];
ary_ret v_14(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      S v_17 = v_10((start_byte_offset + (v_16 * 128u)));
      a[v_16] = v_17;
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  S v_18[4] = a;
  return v_18;
}

[numthreads(1, 1, 1)]
void f() {
  S v_19[4] = v_14(0u);
  a(v_19);
  S v_20 = v_10(256u);
  b(v_20);
  c(v_2(260u));
  d(tint_bitcast_to_f16(u[0u].z).yx);
  e(tint_bitcast_to_f16(u[0u].z).yx[0u]);
}

FXC validation failure:
<scrubbed_path>(3,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
