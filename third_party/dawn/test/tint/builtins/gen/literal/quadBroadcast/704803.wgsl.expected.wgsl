enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn quadBroadcast_704803() -> vec3<i32> {
  var res : vec3<i32> = quadBroadcast(vec3<i32>(1i), 1i);
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
