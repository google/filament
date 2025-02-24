enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn quadBroadcast_f9d579() -> i32 {
  var res : i32 = quadBroadcast(1i, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_f9d579();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_f9d579();
}
