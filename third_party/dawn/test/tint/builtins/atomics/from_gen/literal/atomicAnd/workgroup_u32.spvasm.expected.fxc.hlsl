groupshared uint arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    uint atomic_result = 0u;
    InterlockedExchange(arg_0, 0u, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

static uint local_invocation_index_1 = 0u;

void atomicAnd_34edd3() {
  uint res = 0u;
  uint atomic_result_1 = 0u;
  InterlockedAnd(arg_0, 1u, atomic_result_1);
  uint x_10 = atomic_result_1;
  res = x_10;
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  uint atomic_result_2 = 0u;
  InterlockedExchange(arg_0, 0u, atomic_result_2);
  GroupMemoryBarrierWithGroupSync();
  atomicAnd_34edd3();
  return;
}

void compute_main_1() {
  uint x_30 = local_invocation_index_1;
  compute_main_inner(x_30);
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
