enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupShuffle_4f5711() -> vec3<u32> {
  var res : vec3<u32> = subgroupShuffle(vec3<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_4f5711();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_4f5711();
}
