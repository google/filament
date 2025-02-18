struct S {
  int before;
  float4x2 m;
  int after;
};

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
groupshared S w[4];
float4x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = u[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = u[((16u + start_byte_offset) / 16u)];
  float2 v_6 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = u[((24u + start_byte_offset) / 16u)];
  return float4x2(v_2, v_4, v_6, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

S v_8(uint start_byte_offset) {
  int v_9 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float4x2 v_10 = v((8u + start_byte_offset));
  S v_11 = {v_9, v_10, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_11;
}

typedef S ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      S v_15 = v_8((start_byte_offset + (v_14 * 128u)));
      a[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  S v_16[4] = a;
  return v_16;
}

void f_inner(uint tint_local_index) {
  {
    uint v_17 = 0u;
    v_17 = tint_local_index;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      S v_19 = (S)0;
      w[v_18] = v_19;
      {
        v_17 = (v_18 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_20[4] = v_12(0u);
  w = v_20;
  S v_21 = v_8(256u);
  w[1u] = v_21;
  w[3u].m = v(264u);
  w[1u].m[0u] = asfloat(u[1u].xy).yx;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

