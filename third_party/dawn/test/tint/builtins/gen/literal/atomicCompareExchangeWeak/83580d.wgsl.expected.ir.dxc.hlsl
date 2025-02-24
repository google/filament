struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint arg_0;
void atomicCompareExchangeWeak_83580d() {
  uint v = 0u;
  InterlockedCompareExchange(arg_0, 1u, 1u, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 res = {v_1, (v_1 == 1u)};
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    uint v_2 = 0u;
    InterlockedExchange(arg_0, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_83580d();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

