enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupShuffleUp_bbf7f4() -> f16 {
  var res : f16 = subgroupShuffleUp(1.0h, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_bbf7f4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_bbf7f4();
}
