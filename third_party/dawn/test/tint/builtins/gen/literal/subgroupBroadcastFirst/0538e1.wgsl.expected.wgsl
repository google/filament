enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupBroadcastFirst_0538e1() -> f32 {
  var res : f32 = subgroupBroadcastFirst(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcastFirst_0538e1();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcastFirst_0538e1();
}
