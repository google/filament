enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

fn quadBroadcast_ef7d5d() -> vec3<f16> {
  var arg_0 = vec3<f16>(1.0h);
  const arg_1 = 1u;
  var res : vec3<f16> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_ef7d5d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_ef7d5d();
}
