enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

fn subgroupInclusiveMul_1cdf5c() -> vec4<u32> {
  var res : vec4<u32> = subgroupInclusiveMul(vec4<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveMul_1cdf5c();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveMul_1cdf5c();
}
