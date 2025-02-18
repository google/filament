groupshared uint wg[4];

void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      uint atomic_result = 0u;
      InterlockedExchange(wg[i], 0u, atomic_result);
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint atomic_result_1 = 0u;
  InterlockedExchange(wg[1], 1u, atomic_result_1);
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
