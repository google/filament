struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int arg_0;
void atomicCompareExchangeWeak_e88938() {
  int v = int(0);
  InterlockedCompareExchange(arg_0, int(1), int(1), v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 res = {v_1, (v_1 == int(1))};
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    int v_2 = int(0);
    InterlockedExchange(arg_0, int(0), v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_e88938();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

