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

struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

static uint local_invocation_index_1 = 0u;

void atomicCompareExchangeWeak_e88938() {
  int arg_1 = 0;
  int arg_2 = 0;
  x__atomic_compare_exchange_resulti32 res = (x__atomic_compare_exchange_resulti32)0;
  arg_1 = 1;
  arg_2 = 1;
  int x_22 = arg_2;
  int x_23 = arg_1;
  atomic_compare_exchange_result_i32 atomic_result_1 = (atomic_compare_exchange_result_i32)0;
  int atomic_compare_value = x_23;
  InterlockedCompareExchange(arg_0, atomic_compare_value, x_22, atomic_result_1.old_value);
  atomic_result_1.exchanged = atomic_result_1.old_value == atomic_compare_value;
  atomic_compare_exchange_result_i32 tint_symbol = atomic_result_1;
  int old_value_1 = tint_symbol.old_value;
  int x_24 = old_value_1;
  x__atomic_compare_exchange_resulti32 tint_symbol_3 = {x_24, (x_24 == x_22)};
  res = tint_symbol_3;
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  int atomic_result_2 = 0;
  InterlockedExchange(arg_0, 0, atomic_result_2);
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_e88938();
  return;
}

void compute_main_1() {
  uint x_41 = local_invocation_index_1;
  compute_main_inner(x_41);
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
