groupshared int arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    int atomic_result = 0;
    InterlockedExchange(arg_0, 0, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

RWByteAddressBuffer prevent_dce : register(u0);

int atomicXor_75dc95() {
  int atomic_result_1 = 0;
  InterlockedXor(arg_0, 1, atomic_result_1);
  int res = atomic_result_1;
  return res;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  prevent_dce.Store(0u, asuint(atomicXor_75dc95()));
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
