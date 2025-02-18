enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupExclusiveMul_dc51f8() -> u32 {
  var res : u32 = subgroupExclusiveMul(1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_dc51f8();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_dc51f8();
}
