enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupBroadcast_6290a2() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  const arg_1 = 1i;
  var res : vec4<i32> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_6290a2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_6290a2();
}
