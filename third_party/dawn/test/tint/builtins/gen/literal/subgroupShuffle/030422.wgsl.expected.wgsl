enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupShuffle_030422() -> f32 {
  var res : f32 = subgroupShuffle(1.0f, 1i);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_030422();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_030422();
}
