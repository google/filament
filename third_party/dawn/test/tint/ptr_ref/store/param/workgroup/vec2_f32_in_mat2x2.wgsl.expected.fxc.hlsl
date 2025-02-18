void set_matrix_column(inout float2x2 mat, int col, float2 val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
  }
}

groupshared float2x2 S;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    S = float2x2((0.0f).xx, (0.0f).xx);
  }
  GroupMemoryBarrierWithGroupSync();
}

void func_S_X(uint pointer[1]) {
  set_matrix_column(S, pointer[0], (0.0f).xx);
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint tint_symbol_2[1] = {1u};
  func_S_X(tint_symbol_2);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
