cbuffer cbuffer_tint_symbol_3 : register(b0) {
  uint4 tint_symbol_3[1];
};

struct tint_symbol_1 {
  uint3 local_invocation_id : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 global_invocation_id : SV_DispatchThreadID;
  uint3 workgroup_id : SV_GroupID;
};

void main_inner(uint3 local_invocation_id, uint local_invocation_index, uint3 global_invocation_id, uint3 workgroup_id, uint3 num_workgroups) {
  uint foo = ((((local_invocation_id.x + local_invocation_index) + global_invocation_id.x) + workgroup_id.x) + num_workgroups.x);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_id, tint_symbol.local_invocation_index, tint_symbol.global_invocation_id, tint_symbol.workgroup_id, tint_symbol_3[0].xyz);
  return;
}
