@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn dpdy_a8b56e() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = dpdy(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdy_a8b56e();
}
