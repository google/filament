enable subgroups;

@group(0) @binding(0) var<storage, read_write> output : array<u32>;

struct ComputeInputs {
  @builtin(subgroup_invocation_id)
  subgroup_invocation_id : u32,
  @builtin(subgroup_size)
  subgroup_size : u32,
}

@compute @workgroup_size(1)
fn main(inputs : ComputeInputs) {
  output[inputs.subgroup_invocation_id] = inputs.subgroup_size;
}
