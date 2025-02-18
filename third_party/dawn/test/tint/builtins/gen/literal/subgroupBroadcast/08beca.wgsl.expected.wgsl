enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupBroadcast_08beca() -> f32 {
  var res : f32 = subgroupBroadcast(1.0f, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_08beca();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_08beca();
}
