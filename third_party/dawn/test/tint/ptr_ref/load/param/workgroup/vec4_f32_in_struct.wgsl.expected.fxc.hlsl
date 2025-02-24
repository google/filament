struct str {
  float4 i;
};

groupshared str S;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    str tint_symbol_2 = (str)0;
    S = tint_symbol_2;
  }
  GroupMemoryBarrierWithGroupSync();
}

float4 func_S_i() {
  return S.i;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  float4 r = func_S_i();
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
