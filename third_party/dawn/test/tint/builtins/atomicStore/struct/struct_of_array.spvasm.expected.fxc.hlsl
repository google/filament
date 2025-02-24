struct S_atomic {
  int x;
  uint a[10];
  uint y;
};

groupshared S_atomic wg;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    wg.x = 0;
    wg.y = 0u;
  }
  {
    for(uint idx_1 = local_idx; (idx_1 < 10u); idx_1 = (idx_1 + 1u)) {
      uint i = idx_1;
      uint atomic_result = 0u;
      InterlockedExchange(wg.a[i], 0u, atomic_result);
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

static uint local_invocation_index_1 = 0u;

void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  wg.x = 0;
  wg.y = 0u;
  idx = local_invocation_index_2;
  while (true) {
    if (!((idx < 10u))) {
      break;
    }
    uint x_35 = idx;
    uint atomic_result_1 = 0u;
    InterlockedExchange(wg.a[min(x_35, 9u)], 0u, atomic_result_1);
    {
      idx = (idx + 1u);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_2 = 0u;
  InterlockedExchange(wg.a[4], 1u, atomic_result_2);
  return;
}

void compute_main_1() {
  uint x_53 = local_invocation_index_1;
  compute_main_inner(x_53);
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
