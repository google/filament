struct ComputeInputs0 {
  @builtin(local_invocation_id) local_invocation_id : vec3<u32>,
};
struct ComputeInputs1 {
  @builtin(workgroup_id) workgroup_id : vec3<u32>,
};

@compute @workgroup_size(1)
fn main(
  inputs0 : ComputeInputs0,
  @builtin(local_invocation_index) local_invocation_index : u32,
  @builtin(global_invocation_id) global_invocation_id : vec3<u32>,
  inputs1 : ComputeInputs1,
) {
  let foo : u32 =
    inputs0.local_invocation_id.x +
    local_invocation_index +
    global_invocation_id.x +
    inputs1.workgroup_id.x;
}
