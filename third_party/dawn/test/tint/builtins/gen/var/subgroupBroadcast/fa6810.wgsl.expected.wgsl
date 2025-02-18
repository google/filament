enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupBroadcast_fa6810() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  const arg_1 = 1i;
  var res : vec2<i32> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_fa6810();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_fa6810();
}
