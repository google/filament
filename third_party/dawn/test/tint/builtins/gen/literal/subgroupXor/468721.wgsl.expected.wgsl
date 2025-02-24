enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupXor_468721() -> vec3<u32> {
  var res : vec3<u32> = subgroupXor(vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupXor_468721();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_468721();
}
