groupshared float2x3 w[4];

void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      w[i] = float2x3((0.0f).xxx, (0.0f).xxx);
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

float2x3 u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz));
}

typedef float2x3 u_load_ret[4];
u_load_ret u_load(uint offset) {
  float2x3 arr[4] = (float2x3[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = u_load_1((offset + (i_1 * 32u)));
    }
  }
  return arr;
}

void f_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  w = u_load(0u);
  w[1] = u_load_1(64u);
  w[1][0] = asfloat(u[1].xyz).zxy;
  w[1][0].x = asfloat(u[1].x);
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
