struct F_inputs {
  uint mat2x2 : SV_GroupIndex;
};


groupshared float2x2 W;
void F_inner(uint mat2x2_1) {
  if ((mat2x2_1 < 1u)) {
    W = float2x2((0.0f).xx, (0.0f).xx);
  }
  GroupMemoryBarrierWithGroupSync();
  W[0u] = (W[0u] + 0.0f);
}

[numthreads(1, 1, 1)]
void F(F_inputs inputs) {
  F_inner(inputs.mat2x2);
}

