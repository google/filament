enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupBroadcastFirst_5e5b6f() -> vec3<u32> {
  var arg_0 = vec3<u32>(1u);
  var res : vec3<u32> = subgroupBroadcastFirst(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcastFirst_5e5b6f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcastFirst_5e5b6f();
}
