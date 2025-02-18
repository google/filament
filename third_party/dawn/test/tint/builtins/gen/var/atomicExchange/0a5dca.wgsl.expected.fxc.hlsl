groupshared uint arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    uint atomic_result = 0u;
    InterlockedExchange(arg_0, 0u, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

RWByteAddressBuffer prevent_dce : register(u0);

uint atomicExchange_0a5dca() {
  uint arg_1 = 1u;
  uint atomic_result_1 = 0u;
  InterlockedExchange(arg_0, arg_1, atomic_result_1);
  uint res = atomic_result_1;
  return res;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  prevent_dce.Store(0u, asuint(atomicExchange_0a5dca()));
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
