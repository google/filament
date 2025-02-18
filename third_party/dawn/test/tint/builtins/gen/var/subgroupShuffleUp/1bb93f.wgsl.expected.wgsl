enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupShuffleUp_1bb93f() -> i32 {
  var arg_0 = 1i;
  var arg_1 = 1u;
  var res : i32 = subgroupShuffleUp(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_1bb93f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_1bb93f();
}
