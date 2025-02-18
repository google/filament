enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupMul_3fe886() -> i32 {
  var res : i32 = subgroupMul(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMul_3fe886();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_3fe886();
}
