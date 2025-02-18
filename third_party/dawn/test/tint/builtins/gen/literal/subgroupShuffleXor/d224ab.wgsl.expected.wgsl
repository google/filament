enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupShuffleXor_d224ab() -> f16 {
  var res : f16 = subgroupShuffleXor(1.0h, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_d224ab();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_d224ab();
}
