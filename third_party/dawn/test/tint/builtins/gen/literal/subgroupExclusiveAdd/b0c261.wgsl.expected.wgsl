enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupExclusiveAdd_b0c261() -> i32 {
  var res : i32 = subgroupExclusiveAdd(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveAdd_b0c261();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_b0c261();
}
