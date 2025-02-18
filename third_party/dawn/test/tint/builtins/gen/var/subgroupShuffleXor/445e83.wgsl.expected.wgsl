enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupShuffleXor_445e83() -> i32 {
  var arg_0 = 1i;
  var arg_1 = 1u;
  var res : i32 = subgroupShuffleXor(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_445e83();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_445e83();
}
