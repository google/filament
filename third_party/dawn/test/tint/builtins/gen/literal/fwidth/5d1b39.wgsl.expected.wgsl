@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn fwidth_5d1b39() -> vec3<f32> {
  var res : vec3<f32> = fwidth(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fwidth_5d1b39();
}
