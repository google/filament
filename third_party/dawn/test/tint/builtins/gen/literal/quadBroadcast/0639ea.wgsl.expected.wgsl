enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn quadBroadcast_0639ea() -> i32 {
  var res : i32 = quadBroadcast(1i, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_0639ea();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_0639ea();
}
