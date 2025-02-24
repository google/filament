groupshared int zero[3];

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 3u)) {
    uint i = local_idx;
    zero[i] = 0;
  }
  GroupMemoryBarrierWithGroupSync();
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  int v[3] = zero;
}

[numthreads(10, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
