@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn dpdy_7f8d84() -> f32 {
  var arg_0 = 1.0f;
  var res : f32 = dpdy(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdy_7f8d84();
}
