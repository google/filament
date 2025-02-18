@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn dpdy_a8b56e() -> vec2<f32> {
  var res : vec2<f32> = dpdy(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdy_a8b56e();
}
