struct main_inputs {
  uint3 local_invocation_id : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 global_invocation_id : SV_DispatchThreadID;
  uint3 workgroup_id : SV_GroupID;
};


cbuffer cbuffer_tint_num_workgroups : register(b0) {
  uint4 tint_num_workgroups[1];
};
void main_inner(uint3 local_invocation_id, uint local_invocation_index, uint3 global_invocation_id, uint3 workgroup_id, uint3 num_workgroups) {
  uint foo = ((((local_invocation_id.x + local_invocation_index) + global_invocation_id.x) + workgroup_id.x) + num_workgroups.x);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.local_invocation_id, inputs.local_invocation_index, inputs.global_invocation_id, inputs.workgroup_id, tint_num_workgroups[0u].xyz);
}

