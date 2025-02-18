enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn quadBroadcast_960c6b() -> f32 {
  var res : f32 = quadBroadcast(1.0f, 1u);
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
