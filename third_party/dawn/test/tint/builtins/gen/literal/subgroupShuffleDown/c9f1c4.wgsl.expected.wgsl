enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

fn subgroupShuffleDown_c9f1c4() -> vec2<u32> {
  var res : vec2<u32> = subgroupShuffleDown(vec2<u32>(1u), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_c9f1c4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_c9f1c4();
}
