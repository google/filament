enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupXor_694b17() -> i32 {
  var arg_0 = 1i;
  var res : i32 = subgroupXor(arg_0);
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
