@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn fwidthFine_68f4ef() -> vec4<f32> {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = fwidthFine(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fwidthFine_68f4ef();
}
