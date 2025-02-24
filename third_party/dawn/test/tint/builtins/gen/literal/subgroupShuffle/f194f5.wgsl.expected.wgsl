enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupShuffle_f194f5() -> u32 {
  var res : u32 = subgroupShuffle(1u, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_f194f5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_f194f5();
}
