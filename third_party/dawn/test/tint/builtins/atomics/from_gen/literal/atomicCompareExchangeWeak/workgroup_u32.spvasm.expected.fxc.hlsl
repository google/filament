struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};
groupshared uint arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    uint atomic_result = 0u;
    InterlockedExchange(arg_0, 0u, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

struct x__atomic_compare_exchange_resultu32 {
  uint old_value;
  bool exchanged;
};

static uint local_invocation_index_1 = 0u;

void atomicCompareExchangeWeak_83580d() {
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  atomic_compare_exchange_result_u32 atomic_result_1 = (atomic_compare_exchange_result_u32)0;
  uint atomic_compare_value = 1u;
  InterlockedCompareExchange(arg_0, atomic_compare_value, 1u, atomic_result_1.old_value);
  atomic_result_1.exchanged = atomic_result_1.old_value == atomic_compare_value;
  atomic_compare_exchange_result_u32 tint_symbol = atomic_result_1;
  uint old_value_1 = tint_symbol.old_value;
  uint x_17 = old_value_1;
  x__atomic_compare_exchange_resultu32 tint_symbol_3 = {x_17, (x_17 == 1u)};
  res = tint_symbol_3;
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  uint atomic_result_2 = 0u;
  InterlockedExchange(arg_0, 0u, atomic_result_2);
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_83580d();
  return;
}

void compute_main_1() {
  uint x_35 = local_invocation_index_1;
  compute_main_inner(x_35);
  return;
}

struct tint_symbol_2 {
  uint local_invocation_index_1_param : SV_GroupIndex;
};

void compute_main_inner_1(uint local_invocation_index_1_param) {
  tint_zero_workgroup_memory(local_invocation_index_1_param);
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_2 tint_symbol_1) {
  compute_main_inner_1(tint_symbol_1.local_invocation_index_1_param);
  return;
}
