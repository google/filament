struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
groupshared int arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    int atomic_result = 0;
    InterlockedExchange(arg_0, 0, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

void atomicCompareExchangeWeak_e88938() {
  atomic_compare_exchange_result_i32 atomic_result_1 = (atomic_compare_exchange_result_i32)0;
  int atomic_compare_value = 1;
  InterlockedCompareExchange(arg_0, atomic_compare_value, 1, atomic_result_1.old_value);
  atomic_result_1.exchanged = atomic_result_1.old_value == atomic_compare_value;
  atomic_compare_exchange_result_i32 res = atomic_result_1;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicCompareExchangeWeak_e88938();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
