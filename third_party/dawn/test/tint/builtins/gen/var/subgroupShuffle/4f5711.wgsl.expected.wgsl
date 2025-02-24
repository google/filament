enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupShuffle_4f5711() -> vec3<u32> {
  var arg_0 = vec3<u32>(1u);
  var arg_1 = 1u;
  var res : vec3<u32> = subgroupShuffle(arg_0, arg_1);
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
