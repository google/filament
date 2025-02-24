struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared int arg_0;
void atomicStore_8bea94() {
  int v = int(0);
  InterlockedExchange(arg_0, int(1), v);
}

void compute_main_inner(uint local_invocation_index_2) {
  int v_1 = int(0);
  InterlockedExchange(arg_0, int(0), v_1);
  GroupMemoryBarrierWithGroupSync();
  atomicStore_8bea94();
}

void compute_main_1() {
  uint x_29 = local_invocation_index_1;
  compute_main_inner(x_29);
}

void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    int v_2 = int(0);
    InterlockedExchange(arg_0, int(0), v_2);
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner_1(inputs.local_invocation_index_1_param);
}

