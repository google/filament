struct S {
  int before;
  matrix<float16_t, 3, 4> m;
  int after;
};

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
groupshared S w[4];
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

matrix<float16_t, 3, 4> v_4(uint start_byte_offset) {
  uint4 v_5 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint4 v_9 = u[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 4>(v_6, v_8, tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy))));
}

S v_10(uint start_byte_offset) {
  int v_11 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  matrix<float16_t, 3, 4> v_12 = v_4((8u + start_byte_offset));
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

void f_inner(uint tint_local_index) {
  {
    uint v_19 = 0u;
    v_19 = tint_local_index;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      S v_21 = (S)0;
      w[v_20] = v_21;
      {
        v_19 = (v_20 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_22[4] = v_14(0u);
  w = v_22;
  S v_23 = v_10(256u);
  w[1u] = v_23;
  w[3u].m = v_4(264u);
  w[1u].m[0u] = tint_bitcast_to_f16(u[1u].xy).ywxz;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

