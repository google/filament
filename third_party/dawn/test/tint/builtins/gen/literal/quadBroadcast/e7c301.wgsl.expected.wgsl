enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

fn quadBroadcast_e7c301() -> vec4<f16> {
  var res : vec4<f16> = quadBroadcast(vec4<f16>(1.0h), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_e7c301();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_e7c301();
}
