enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn quadBroadcast_2d0b7d() -> vec4<u32> {
  var res : vec4<u32> = quadBroadcast(vec4<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_2d0b7d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_2d0b7d();
}
