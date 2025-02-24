enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

fn subgroupMul_fa781b() -> vec3<u32> {
  var res : vec3<u32> = subgroupMul(vec3<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMul_fa781b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_fa781b();
}
