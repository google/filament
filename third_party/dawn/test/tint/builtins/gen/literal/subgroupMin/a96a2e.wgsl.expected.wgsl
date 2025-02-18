enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupMin_a96a2e() -> i32 {
  var res : i32 = subgroupMin(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_a96a2e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_a96a2e();
}
