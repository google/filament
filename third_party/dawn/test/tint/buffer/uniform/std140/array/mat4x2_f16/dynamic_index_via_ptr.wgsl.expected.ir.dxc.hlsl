
cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(a[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(a[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(a[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 4, 2>(v_3, v_4, v_5, tint_bitcast_to_f16(a[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a_1[4] = (matrix<float16_t, 4, 2>[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a_1[v_8] = v_2((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_9[4] = a_1;
  return v_9;
}

[numthreads(1, 1, 1)]
void f() {
  uint v_10 = (16u * min(uint(i()), 3u));
  uint v_11 = (4u * min(uint(i()), 3u));
  matrix<float16_t, 4, 2> l_a[4] = v_6(0u);
  matrix<float16_t, 4, 2> l_a_i = v_2(v_10);
  vector<float16_t, 2> l_a_i_i = tint_bitcast_to_f16(a[((v_10 + v_11) / 16u)][(((v_10 + v_11) % 16u) / 4u)]);
  uint v_12 = a[((v_10 + v_11) / 16u)][(((v_10 + v_11) % 16u) / 4u)];
  s.Store<float16_t>(0u, (((float16_t(f16tof32((v_12 >> (((((v_10 + v_11) % 4u) == 0u)) ? (0u) : (16u))))) + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x));
}

