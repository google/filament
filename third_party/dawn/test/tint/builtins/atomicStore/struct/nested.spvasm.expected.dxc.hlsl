struct S0_atomic {
  int x;
  uint a;
  int y;
  int z;
};
struct S1_atomic {
  int x;
  S0_atomic a;
  int y;
  int z;
};
struct S2_atomic {
  int x;
  int y;
  int z;
  S1_atomic a;
};

groupshared S2_atomic wg;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    wg.x = 0;
    wg.y = 0;
    wg.z = 0;
    wg.a.x = 0;
    wg.a.a.x = 0;
    uint atomic_result = 0u;
    InterlockedExchange(wg.a.a.a, 0u, atomic_result);
    wg.a.a.y = 0;
    wg.a.a.z = 0;
    wg.a.y = 0;
    wg.a.z = 0;
  }
  GroupMemoryBarrierWithGroupSync();
}

static uint local_invocation_index_1 = 0u;

void compute_main_inner(uint local_invocation_index_2) {
  wg.x = 0;
  wg.y = 0;
  wg.z = 0;
  wg.a.x = 0;
  wg.a.a.x = 0;
  uint atomic_result_1 = 0u;
  InterlockedExchange(wg.a.a.a, 0u, atomic_result_1);
  wg.a.a.y = 0;
  wg.a.a.z = 0;
  wg.a.y = 0;
  wg.a.z = 0;
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_2 = 0u;
  InterlockedExchange(wg.a.a.a, 1u, atomic_result_2);
  return;
}

void compute_main_1() {
  uint x_44 = local_invocation_index_1;
  compute_main_inner(x_44);
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
