enable subgroups;

@group(0) @binding(0)
var<storage, read_write> output: array<u32>;

@fragment
fn main(
  @builtin(subgroup_invocation_id) subgroup_invocation_id : u32,
  @builtin(subgroup_size) subgroup_size : u32,
) {
  output[subgroup_invocation_id] = subgroup_size;
}
