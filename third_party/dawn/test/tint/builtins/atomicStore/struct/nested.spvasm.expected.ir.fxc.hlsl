struct S0_atomic {
  int x;
  uint a;
  int y;
  int z;
};

struct S1_atomic {
  int x;
  S0_atomic a;
  int y;
  int z;
};

struct S2_atomic {
  int x;
  int y;
  int z;
  S1_atomic a;
};

struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared S2_atomic wg;
void compute_main_inner(uint local_invocation_index_2) {
  wg.x = int(0);
  wg.y = int(0);
  wg.z = int(0);
  wg.a.x = int(0);
  wg.a.a.x = int(0);
  uint v = 0u;
  InterlockedExchange(wg.a.a.a, 0u, v);
  wg.a.a.y = int(0);
  wg.a.a.z = int(0);
  wg.a.y = int(0);
  wg.a.z = int(0);
  GroupMemoryBarrierWithGroupSync();
  uint v_1 = 0u;
  InterlockedExchange(wg.a.a.a, 1u, v_1);
}

void compute_main_1() {
  uint x_44 = local_invocation_index_1;
  compute_main_inner(x_44);
}

void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    wg.x = int(0);
    wg.y = int(0);
    wg.z = int(0);
    wg.a.x = int(0);
    wg.a.a.x = int(0);
    uint v_2 = 0u;
    InterlockedExchange(wg.a.a.a, 0u, v_2);
    wg.a.a.y = int(0);
    wg.a.a.z = int(0);
    wg.a.y = int(0);
    wg.a.z = int(0);
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner_1(inputs.local_invocation_index_1_param);
}

