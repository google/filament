struct ComputeInputs0 {
  uint3 local_invocation_id;
};

struct ComputeInputs1 {
  uint3 workgroup_id;
};

struct main_inputs {
  uint3 ComputeInputs0_local_invocation_id : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 global_invocation_id : SV_DispatchThreadID;
  uint3 ComputeInputs1_workgroup_id : SV_GroupID;
};


void main_inner(ComputeInputs0 inputs0, uint local_invocation_index, uint3 global_invocation_id, ComputeInputs1 inputs1) {
  uint foo = (((inputs0.local_invocation_id.x + local_invocation_index) + global_invocation_id.x) + inputs1.workgroup_id.x);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  ComputeInputs0 v = {inputs.ComputeInputs0_local_invocation_id};
  ComputeInputs1 v_1 = {inputs.ComputeInputs1_workgroup_id};
  main_inner(v, inputs.local_invocation_index, inputs.global_invocation_id, v_1);
}

