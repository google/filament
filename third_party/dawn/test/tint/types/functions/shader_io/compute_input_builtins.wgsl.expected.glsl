#version 310 es

void main_inner(uvec3 local_invocation_id, uint local_invocation_index, uvec3 global_invocation_id, uvec3 workgroup_id, uvec3 num_workgroups) {
  uint foo = ((((local_invocation_id.x + local_invocation_index) + global_invocation_id.x) + workgroup_id.x) + num_workgroups.x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationID, gl_LocalInvocationIndex, gl_GlobalInvocationID, gl_WorkGroupID, gl_NumWorkGroups);
}
