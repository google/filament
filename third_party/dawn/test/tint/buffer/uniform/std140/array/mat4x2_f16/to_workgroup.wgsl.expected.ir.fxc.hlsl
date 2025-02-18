SKIP: INVALID

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
groupshared matrix<float16_t, 4, 2> w[4];
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
      a[v_12] = v_2((start_byte_offset + (v_12 * 16u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_13[4] = a;
  return v_13;
}

void f_inner(uint tint_local_index) {
  {
    uint v_14 = 0u;
    v_14 = tint_local_index;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      w[v_15] = matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  matrix<float16_t, 4, 2> v_16[4] = v_10(0u);
  w = v_16;
  w[int(1)] = v_2(32u);
  w[int(1)][int(0)] = tint_bitcast_to_f16(u[0u].x).yx;
  w[int(1)][int(0)][0u] = float16_t(f16tof32(u[0u].y));
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

FXC validation failure:
<scrubbed_path>(9,20-28): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
