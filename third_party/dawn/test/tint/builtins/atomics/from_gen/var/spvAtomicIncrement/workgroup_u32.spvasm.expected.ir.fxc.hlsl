struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared uint arg_0;
void atomicAdd_d5db1d() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  uint v = 0u;
  InterlockedAdd(arg_0, 1u, v);
  uint x_14 = v;
  res = x_14;
}

void compute_main_inner(uint local_invocation_index_2) {
  uint v_1 = 0u;
  InterlockedExchange(arg_0, 0u, v_1);
  GroupMemoryBarrierWithGroupSync();
  atomicAdd_d5db1d();
}

void compute_main_1() {
  uint x_32 = local_invocation_index_1;
  compute_main_inner(x_32);
}

void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    uint v_2 = 0u;
    InterlockedExchange(arg_0, 0u, v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner_1(inputs.local_invocation_index_1_param);
}

