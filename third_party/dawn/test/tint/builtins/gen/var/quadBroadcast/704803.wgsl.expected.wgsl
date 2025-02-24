enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn quadBroadcast_704803() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  const arg_1 = 1i;
  var res : vec3<i32> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_704803();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_704803();
}
