enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn quadBroadcast_b68331() -> vec4<u32> {
  var arg_0 = vec4<u32>(1u);
  const arg_1 = 1i;
  var res : vec4<u32> = quadBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_b68331();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_b68331();
}
