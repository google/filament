enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupShuffleUp_3242a6() -> u32 {
  var arg_0 = 1u;
  var arg_1 = 1u;
  var res : u32 = subgroupShuffleUp(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_3242a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_3242a6();
}
