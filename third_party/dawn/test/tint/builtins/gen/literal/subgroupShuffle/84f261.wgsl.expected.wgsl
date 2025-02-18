enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupShuffle_84f261() -> vec4<u32> {
  var res : vec4<u32> = subgroupShuffle(vec4<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_84f261();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_84f261();
}
