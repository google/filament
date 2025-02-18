@compute @workgroup_size(1)
fn main(@builtin(local_invocation_id) local_invocation_id : vec3<u32>, @builtin(local_invocation_index) local_invocation_index : u32, @builtin(global_invocation_id) global_invocation_id : vec3<u32>, @builtin(workgroup_id) workgroup_id : vec3<u32>, @builtin(num_workgroups) num_workgroups : vec3<u32>) {
  let foo : u32 = ((((local_invocation_id.x + local_invocation_index) + global_invocation_id.x) + workgroup_id.x) + num_workgroups.x);
}
