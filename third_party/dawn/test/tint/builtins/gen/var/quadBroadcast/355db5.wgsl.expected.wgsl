enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn quadBroadcast_355db5() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  const arg_1 = 1i;
  var res : vec3<f32> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_355db5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_355db5();
}
