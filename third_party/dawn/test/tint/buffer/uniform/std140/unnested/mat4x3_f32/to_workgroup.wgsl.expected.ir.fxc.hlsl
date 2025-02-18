struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
groupshared float4x3 w;
float4x3 v(uint start_byte_offset) {
  return float4x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz), asfloat(u[((32u + start_byte_offset) / 16u)].xyz), asfloat(u[((48u + start_byte_offset) / 16u)].xyz));
}

void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = v(0u);
  w[1u] = asfloat(u[0u].xyz);
  w[1u] = asfloat(u[0u].xyz).zxy;
  w[0u].y = asfloat(u[1u].x);
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

