@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn fwidthCoarse_4e4fc4() -> vec4<f32> {
  var res : vec4<f32> = fwidthCoarse(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fwidthCoarse_4e4fc4();
}
