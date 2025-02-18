enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupXor_694b17() -> i32 {
  var res : i32 = subgroupXor(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupXor_694b17();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupXor_694b17();
}
