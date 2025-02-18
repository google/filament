enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupShuffleUp_3e609f() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  var arg_1 = 1u;
  var res : vec4<i32> = subgroupShuffleUp(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_3e609f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_3e609f();
}
