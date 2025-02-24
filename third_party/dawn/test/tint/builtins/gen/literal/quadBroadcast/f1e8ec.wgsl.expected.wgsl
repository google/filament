enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn quadBroadcast_f1e8ec() -> vec3<u32> {
  var res : vec3<u32> = quadBroadcast(vec3<u32>(1u), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_f1e8ec();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_f1e8ec();
}
