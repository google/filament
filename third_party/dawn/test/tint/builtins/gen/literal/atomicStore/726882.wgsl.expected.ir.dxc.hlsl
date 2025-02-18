struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint arg_0;
void atomicStore_726882() {
  uint v = 0u;
  InterlockedExchange(arg_0, 1u, v);
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    uint v_1 = 0u;
    InterlockedExchange(arg_0, 0u, v_1);
  }
  GroupMemoryBarrierWithGroupSync();
  atomicStore_726882();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

