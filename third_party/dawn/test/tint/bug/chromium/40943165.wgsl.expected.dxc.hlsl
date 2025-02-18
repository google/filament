groupshared float2x2 W;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    W = float2x2((0.0f).xx, (0.0f).xx);
  }
  GroupMemoryBarrierWithGroupSync();
}

struct tint_symbol_2 {
  uint mat2x2 : SV_GroupIndex;
};

void F_inner(uint mat2x2) {
  tint_zero_workgroup_memory(mat2x2);
  W[0] = (W[0] + 0.0f);
}

[numthreads(1, 1, 1)]
void F(tint_symbol_2 tint_symbol_1) {
  F_inner(tint_symbol_1.mat2x2);
  return;
}
