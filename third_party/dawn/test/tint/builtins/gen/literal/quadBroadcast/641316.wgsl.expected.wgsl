enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn quadBroadcast_641316() -> vec2<u32> {
  var res : vec2<u32> = quadBroadcast(vec2<u32>(1u), 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_641316();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_641316();
}
