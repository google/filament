enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupAnd_c6fc92() -> i32 {
  var res : i32 = subgroupAnd(1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAnd_c6fc92();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAnd_c6fc92();
}
