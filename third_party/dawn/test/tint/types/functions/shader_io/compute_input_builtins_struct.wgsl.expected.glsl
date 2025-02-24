#version 310 es


struct ComputeInputs {
  uvec3 local_invocation_id;
  uint local_invocation_index;
  uvec3 global_invocation_id;
  uvec3 workgroup_id;
  uvec3 num_workgroups;
};

void main_inner(ComputeInputs inputs) {
  uint foo = ((((inputs.local_invocation_id.x + inputs.local_invocation_index) + inputs.global_invocation_id.x) + inputs.workgroup_id.x) + inputs.num_workgroups.x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(ComputeInputs(gl_LocalInvocationID, gl_LocalInvocationIndex, gl_GlobalInvocationID, gl_WorkGroupID, gl_NumWorkGroups));
}
