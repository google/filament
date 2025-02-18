struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared float2x2 S;
void func(uint pointer_indices[1]) {
  S[pointer_indices[0u]] = (0.0f).xx;
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    S = float2x2((0.0f).xx, (0.0f).xx);
  }
  GroupMemoryBarrierWithGroupSync();
  uint v[1] = {1u};
  func(v);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

