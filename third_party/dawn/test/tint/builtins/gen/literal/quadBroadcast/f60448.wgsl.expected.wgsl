enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn quadBroadcast_f60448() -> vec2<u32> {
  var res : vec2<u32> = quadBroadcast(vec2<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadBroadcast_f60448();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadBroadcast_f60448();
}
