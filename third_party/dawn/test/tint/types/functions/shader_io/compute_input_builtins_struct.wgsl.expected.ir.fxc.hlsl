struct ComputeInputs {
  uint3 local_invocation_id;
  uint local_invocation_index;
  uint3 global_invocation_id;
  uint3 workgroup_id;
  uint3 num_workgroups;
};

struct main_inputs {
  uint3 ComputeInputs_local_invocation_id : SV_GroupThreadID;
  uint ComputeInputs_local_invocation_index : SV_GroupIndex;
  uint3 ComputeInputs_global_invocation_id : SV_DispatchThreadID;
  uint3 ComputeInputs_workgroup_id : SV_GroupID;
};


cbuffer cbuffer_tint_num_workgroups : register(b0) {
  uint4 tint_num_workgroups[1];
};
void main_inner(ComputeInputs inputs) {
  uint foo = ((((inputs.local_invocation_id.x + inputs.local_invocation_index) + inputs.global_invocation_id.x) + inputs.workgroup_id.x) + inputs.num_workgroups.x);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  ComputeInputs v = {inputs.ComputeInputs_local_invocation_id, inputs.ComputeInputs_local_invocation_index, inputs.ComputeInputs_global_invocation_id, inputs.ComputeInputs_workgroup_id, tint_num_workgroups[0u].xyz};
  main_inner(v);
}

