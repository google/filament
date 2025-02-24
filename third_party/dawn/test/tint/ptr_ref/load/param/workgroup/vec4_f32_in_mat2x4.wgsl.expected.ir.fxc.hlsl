struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared float2x4 S;
float4 func(uint pointer_indices[1]) {
  return S[pointer_indices[0u]];
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    S = float2x4((0.0f).xxxx, (0.0f).xxxx);
  }
  GroupMemoryBarrierWithGroupSync();
  uint v[1] = {1u};
  float4 r = func(v);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

