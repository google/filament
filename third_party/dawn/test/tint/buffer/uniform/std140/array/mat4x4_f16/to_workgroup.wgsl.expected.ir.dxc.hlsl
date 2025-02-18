struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
groupshared matrix<float16_t, 4, 4> w[4];
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

matrix<float16_t, 4, 4> v_4(uint start_byte_offset) {
  uint4 v_5 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint4 v_9 = u[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_10 = tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy)));
  uint4 v_11 = u[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 4>(v_6, v_8, v_10, tint_bitcast_to_f16(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_11.zw) : (v_11.xy))));
}

typedef matrix<float16_t, 4, 4> ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  matrix<float16_t, 4, 4> a[4] = (matrix<float16_t, 4, 4>[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a[v_14] = v_4((start_byte_offset + (v_14 * 32u)));
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 4> v_15[4] = a;
  return v_15;
}

void f_inner(uint tint_local_index) {
  {
    uint v_16 = 0u;
    v_16 = tint_local_index;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      w[v_17] = matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
      {
        v_16 = (v_17 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  matrix<float16_t, 4, 4> v_18[4] = v_12(0u);
  w = v_18;
  w[1u] = v_4(64u);
  w[1u][0u] = tint_bitcast_to_f16(u[0u].zw).ywxz;
  w[1u][0u].x = float16_t(f16tof32(u[0u].z));
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

