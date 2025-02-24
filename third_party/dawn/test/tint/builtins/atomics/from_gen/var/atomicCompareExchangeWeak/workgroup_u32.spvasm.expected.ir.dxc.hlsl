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
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  arg_1 = 1u;
  arg_2 = 1u;
  uint x_21 = arg_2;
  uint x_22 = arg_1;
  uint v = 0u;
  InterlockedCompareExchange(arg_0, x_22, x_21, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 v_2 = {v_1, (v_1 == x_22)};
  uint old_value_1 = v_2.old_value;
  uint x_23 = old_value_1;
  x__atomic_compare_exchange_resultu32 v_3 = {x_23, (x_23 == x_21)};
  res = v_3;
}

void compute_main_inner(uint local_invocation_index_2) {
  uint v_4 = 0u;
  InterlockedExchange(arg_0, 0u, v_4);
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_83580d();
}

void compute_main_1() {
  uint x_40 = local_invocation_index_1;
  compute_main_inner(x_40);
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

