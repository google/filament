enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn quadBroadcast_a2d2b4() -> u32 {
  var arg_0 = 1u;
  const arg_1 = 1i;
  var res : u32 = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_a2d2b4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_a2d2b4();
}
