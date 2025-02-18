enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn quadBroadcast_960c6b() -> f32 {
  var arg_0 = 1.0f;
  const arg_1 = 1u;
  var res : f32 = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_960c6b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_960c6b();
}
