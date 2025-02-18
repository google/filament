struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
groupshared float2x3 w[4];
float2x3 v(uint start_byte_offset) {
  return float2x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz));
}

typedef float2x3 ary_ret[4];
ary_ret v_1(uint start_byte_offset) {
  float2x3 a[4] = (float2x3[4])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      a[v_3] = v((start_byte_offset + (v_3 * 32u)));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float2x3 v_4[4] = a;
  return v_4;
}

void f_inner(uint tint_local_index) {
  {
    uint v_5 = 0u;
    v_5 = tint_local_index;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      w[v_6] = float2x3((0.0f).xxx, (0.0f).xxx);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  float2x3 v_7[4] = v_1(0u);
  w = v_7;
  w[1u] = v(64u);
  w[1u][0u] = asfloat(u[1u].xyz).zxy;
  w[1u][0u].x = asfloat(u[1u].x);
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

