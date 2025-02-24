@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn fwidthFine_523fdc() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = fwidthFine(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fwidthFine_523fdc();
}
