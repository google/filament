struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared int arg_0;
void atomicCompareExchangeWeak_e88938() {
  x__atomic_compare_exchange_resulti32 res = (x__atomic_compare_exchange_resulti32)0;
  int v = int(0);
  InterlockedCompareExchange(arg_0, int(1), int(1), v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 v_2 = {v_1, (v_1 == int(1))};
  int old_value_1 = v_2.old_value;
  int x_18 = old_value_1;
  x__atomic_compare_exchange_resulti32 v_3 = {x_18, (x_18 == int(1))};
  res = v_3;
}

void compute_main_inner(uint local_invocation_index_2) {
  int v_4 = int(0);
  InterlockedExchange(arg_0, int(0), v_4);
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_e88938();
}

void compute_main_1() {
  uint x_36 = local_invocation_index_1;
  compute_main_inner(x_36);
}

void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    int v_5 = int(0);
    InterlockedExchange(arg_0, int(0), v_5);
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner_1(inputs.local_invocation_index_1_param);
}

