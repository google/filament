struct S {
  uint3 v;
  uint u;
};

groupshared S wgvar;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    wgvar.v = (0u).xxx;
    uint atomic_result = 0u;
    InterlockedExchange(wgvar.u, 0u, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

RWByteAddressBuffer output : register(u0);

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void outputatomicStore(uint offset, uint value) {
  uint ignored;
  output.InterlockedExchange(offset, value, ignored);
}


void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint atomic_result_1 = 0u;
  InterlockedOr(wgvar.u, 0, atomic_result_1);
  uint x = atomic_result_1;
  outputatomicStore(12u, x);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
