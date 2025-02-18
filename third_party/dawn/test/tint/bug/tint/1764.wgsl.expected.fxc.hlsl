groupshared int W[246];

void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 246u); idx = (idx + 1u)) {
      uint i = idx;
      W[i] = 0;
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  W[0] = 42;
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
