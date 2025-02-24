struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared int arg_0;
void atomicSub_77883a() {
  int res = int(0);
  int v = int(0);
  InterlockedAdd(arg_0, (int(0) - int(1)), v);
  int x_11 = v;
  res = x_11;
}

void compute_main_inner(uint local_invocation_index_2) {
  int v_1 = int(0);
  InterlockedExchange(arg_0, int(0), v_1);
  GroupMemoryBarrierWithGroupSync();
  atomicSub_77883a();
}

void compute_main_1() {
  uint x_31 = local_invocation_index_1;
  compute_main_inner(x_31);
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

