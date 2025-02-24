enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn quadBroadcast_cebc6a() -> f16 {
  var res : f16 = quadBroadcast(1.0h, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_cebc6a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_cebc6a();
}
