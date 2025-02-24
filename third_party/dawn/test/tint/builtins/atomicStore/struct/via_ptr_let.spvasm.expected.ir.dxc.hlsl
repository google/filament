struct S_atomic {
  int x;
  uint a;
  uint y;
};

struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared S_atomic wg;
void compute_main_inner(uint local_invocation_index_2) {
  wg.x = int(0);
  uint v = 0u;
  InterlockedExchange(wg.a, 0u, v);
  wg.y = 0u;
  GroupMemoryBarrierWithGroupSync();
  uint v_1 = 0u;
  InterlockedExchange(wg.a, 1u, v_1);
}

void compute_main_1() {
  uint x_35 = local_invocation_index_1;
  compute_main_inner(x_35);
}

void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    wg.x = int(0);
    uint v_2 = 0u;
    InterlockedExchange(wg.a, 0u, v_2);
    wg.y = 0u;
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner_1(inputs.local_invocation_index_1_param);
}

