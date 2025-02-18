enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupBroadcast_4a4334() -> vec2<u32> {
  var res : vec2<u32> = subgroupBroadcast(vec2<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_4a4334();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_4a4334();
}
