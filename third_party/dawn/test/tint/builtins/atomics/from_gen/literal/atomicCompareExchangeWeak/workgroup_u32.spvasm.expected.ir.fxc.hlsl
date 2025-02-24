struct x__atomic_compare_exchange_resultu32 {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared uint arg_0;
void atomicCompareExchangeWeak_83580d() {
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  uint v = 0u;
  InterlockedCompareExchange(arg_0, 1u, 1u, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 v_2 = {v_1, (v_1 == 1u)};
  uint old_value_1 = v_2.old_value;
  uint x_17 = old_value_1;
  x__atomic_compare_exchange_resultu32 v_3 = {x_17, (x_17 == 1u)};
  res = v_3;
}

void compute_main_inner(uint local_invocation_index_2) {
  uint v_4 = 0u;
  InterlockedExchange(arg_0, 0u, v_4);
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_83580d();
}

void compute_main_1() {
  uint x_35 = local_invocation_index_1;
  compute_main_inner(x_35);
}

void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    uint v_5 = 0u;
    InterlockedExchange(arg_0, 0u, v_5);
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner_1(inputs.local_invocation_index_1_param);
}

