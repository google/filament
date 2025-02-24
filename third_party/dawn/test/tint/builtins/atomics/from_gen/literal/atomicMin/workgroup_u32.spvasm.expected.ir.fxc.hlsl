struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared uint arg_0;
void atomicMin_69d383() {
  uint res = 0u;
  uint v = 0u;
  InterlockedMin(arg_0, 1u, v);
  uint x_10 = v;
  res = x_10;
}

void compute_main_inner(uint local_invocation_index_2) {
  uint v_1 = 0u;
  InterlockedExchange(arg_0, 0u, v_1);
  GroupMemoryBarrierWithGroupSync();
  atomicMin_69d383();
}

void compute_main_1() {
  uint x_30 = local_invocation_index_1;
  compute_main_inner(x_30);
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

