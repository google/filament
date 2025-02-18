enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn quadBroadcast_c0e704() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  const arg_1 = 1i;
  var res : vec2<i32> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_c0e704();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_c0e704();
}
