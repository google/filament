struct ComputeInputs0 {
  uint3 local_invocation_id;
};
struct ComputeInputs1 {
  uint3 workgroup_id;
};
struct tint_symbol_1 {
  uint3 local_invocation_id : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 global_invocation_id : SV_DispatchThreadID;
  uint3 workgroup_id : SV_GroupID;
};

void main_inner(ComputeInputs0 inputs0, uint local_invocation_index, uint3 global_invocation_id, ComputeInputs1 inputs1) {
  uint foo = (((inputs0.local_invocation_id.x + local_invocation_index) + global_invocation_id.x) + inputs1.workgroup_id.x);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  ComputeInputs0 tint_symbol_2 = {tint_symbol.local_invocation_id};
  ComputeInputs1 tint_symbol_3 = {tint_symbol.workgroup_id};
  main_inner(tint_symbol_2, tint_symbol.local_invocation_index, tint_symbol.global_invocation_id, tint_symbol_3);
  return;
}
