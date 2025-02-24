enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : i32;

fn subgroupShuffle_8bfbcd() -> i32 {
  var res : i32 = subgroupShuffle(1i, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_8bfbcd();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_8bfbcd();
}
