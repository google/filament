struct S_atomic {
  int x;
  uint a;
  uint b;
};

groupshared S_atomic wg;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    wg.x = 0;
    uint atomic_result = 0u;
    InterlockedExchange(wg.a, 0u, atomic_result);
    uint atomic_result_1 = 0u;
    InterlockedExchange(wg.b, 0u, atomic_result_1);
  }
  GroupMemoryBarrierWithGroupSync();
}

static uint local_invocation_index_1 = 0u;

void compute_main_inner(uint local_invocation_index_2) {
  wg.x = 0;
  uint atomic_result_2 = 0u;
  InterlockedExchange(wg.a, 0u, atomic_result_2);
  uint atomic_result_3 = 0u;
  InterlockedExchange(wg.b, 0u, atomic_result_3);
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_4 = 0u;
  InterlockedExchange(wg.a, 1u, atomic_result_4);
  uint atomic_result_5 = 0u;
  InterlockedExchange(wg.b, 2u, atomic_result_5);
  return;
}

void compute_main_1() {
  uint x_39 = local_invocation_index_1;
  compute_main_inner(x_39);
  return;
}

struct tint_symbol_1 {
  uint local_invocation_index_1_param : SV_GroupIndex;
};

void compute_main_inner_1(uint local_invocation_index_1_param) {
  tint_zero_workgroup_memory(local_invocation_index_1_param);
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner_1(tint_symbol.local_invocation_index_1_param);
  return;
}
