struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[3];
};
groupshared float3x4 w;
float3x4 v(uint start_byte_offset) {
  return float3x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]), asfloat(u[((32u + start_byte_offset) / 16u)]));
}

void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = v(0u);
  w[1u] = asfloat(u[0u]);
  w[1u] = asfloat(u[0u]).ywxz;
  w[0u].y = asfloat(u[1u].x);
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

