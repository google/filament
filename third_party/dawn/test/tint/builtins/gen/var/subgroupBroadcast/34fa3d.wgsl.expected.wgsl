enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupBroadcast_34fa3d() -> vec3<u32> {
  var arg_0 = vec3<u32>(1u);
  const arg_1 = 1u;
  var res : vec3<u32> = subgroupBroadcast(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupBroadcast_34fa3d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupBroadcast_34fa3d();
}
