enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupShuffleUp_db5bcb() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var arg_1 = 1u;
  var res : vec2<i32> = subgroupShuffleUp(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_db5bcb();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_db5bcb();
}
