enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupShuffle_f194f5() -> u32 {
  var arg_0 = 1u;
  var arg_1 = 1u;
  var res : u32 = subgroupShuffle(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_f194f5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_f194f5();
}
