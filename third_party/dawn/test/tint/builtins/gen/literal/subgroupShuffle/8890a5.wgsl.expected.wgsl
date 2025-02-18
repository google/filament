enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupShuffle_8890a5() -> vec2<f32> {
  var res : vec2<f32> = subgroupShuffle(vec2<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_8890a5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_8890a5();
}
