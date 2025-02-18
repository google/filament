groupshared int arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    int atomic_result = 0;
    InterlockedExchange(arg_0, 0, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

static uint local_invocation_index_1 = 0u;

void atomicOr_d09248() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  int x_19 = arg_1;
  int atomic_result_1 = 0;
  InterlockedOr(arg_0, x_19, atomic_result_1);
  int x_15 = atomic_result_1;
  res = x_15;
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  int atomic_result_2 = 0;
  InterlockedExchange(arg_0, 0, atomic_result_2);
  GroupMemoryBarrierWithGroupSync();
  atomicOr_d09248();
  return;
}

void compute_main_1() {
  uint x_33 = local_invocation_index_1;
  compute_main_inner(x_33);
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
