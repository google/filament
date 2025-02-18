enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupShuffleDown_5d8b9f() -> vec3<f32> {
  var res : vec3<f32> = subgroupShuffleDown(vec3<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleDown_5d8b9f();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleDown_5d8b9f();
}
