SKIP: INVALID

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
groupshared matrix<float16_t, 2, 4> w[4];
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

matrix<float16_t, 2, 4> v_4(uint start_byte_offset) {
  vector<float16_t, 4> v_5 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]);
  return matrix<float16_t, 2, 4>(v_5, tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]));
}

typedef matrix<float16_t, 2, 4> ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  matrix<float16_t, 2, 4> a[4] = (matrix<float16_t, 2, 4>[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_4((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 2, 4> v_9[4] = a;
  return v_9;
}

void f_inner(uint tint_local_index) {
  {
    uint v_10 = 0u;
    v_10 = tint_local_index;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      w[v_11] = matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  matrix<float16_t, 2, 4> v_12[4] = v_6(0u);
  w = v_12;
  w[int(1)] = v_4(32u);
  w[int(1)][int(0)] = tint_bitcast_to_f16(u[0u]).ywxz;
  w[int(1)][int(0)][0u] = float16_t(f16tof32(u[0u].z));
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

FXC validation failure:
<scrubbed_path>(9,20-28): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
