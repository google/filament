@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn fwidthCoarse_1e59d9() -> vec3<f32> {
  var res : vec3<f32> = fwidthCoarse(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fwidthCoarse_1e59d9();
}
