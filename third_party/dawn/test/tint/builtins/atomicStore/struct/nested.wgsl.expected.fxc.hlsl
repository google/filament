struct S0 {
  int x;
  uint a;
  int y;
  int z;
};
struct S1 {
  int x;
  S0 a;
  int y;
  int z;
};
struct S2 {
  int x;
  int y;
  int z;
  S1 a;
};

groupshared S2 wg;

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

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint atomic_result_1 = 0u;
  InterlockedExchange(wg.a.a.a, 1u, atomic_result_1);
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
