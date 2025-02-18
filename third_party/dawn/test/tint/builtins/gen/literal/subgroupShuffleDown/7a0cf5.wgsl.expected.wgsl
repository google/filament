enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupShuffleDown_7a0cf5() -> vec2<f32> {
  var res : vec2<f32> = subgroupShuffleDown(vec2<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_7a0cf5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_7a0cf5();
}
