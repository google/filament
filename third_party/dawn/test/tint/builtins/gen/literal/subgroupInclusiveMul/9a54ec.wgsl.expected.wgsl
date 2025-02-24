enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupInclusiveMul_9a54ec() -> i32 {
  var res : i32 = subgroupInclusiveMul(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveMul_9a54ec();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveMul_9a54ec();
}
