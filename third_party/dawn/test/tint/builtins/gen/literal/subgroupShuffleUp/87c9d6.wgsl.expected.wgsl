enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupShuffleUp_87c9d6() -> vec3<f32> {
  var res : vec3<f32> = subgroupShuffleUp(vec3<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleUp_87c9d6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleUp_87c9d6();
}
