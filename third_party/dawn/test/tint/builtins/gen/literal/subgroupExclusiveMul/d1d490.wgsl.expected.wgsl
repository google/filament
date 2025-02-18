enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupExclusiveMul_d1d490() -> vec2<u32> {
  var res : vec2<u32> = subgroupExclusiveMul(vec2<u32>(1u));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_d1d490();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_d1d490();
}
