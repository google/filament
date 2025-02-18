enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupShuffle_85587b() -> vec4<f32> {
  var res : vec4<f32> = subgroupShuffle(vec4<f32>(1.0f), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffle_85587b();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffle_85587b();
}
