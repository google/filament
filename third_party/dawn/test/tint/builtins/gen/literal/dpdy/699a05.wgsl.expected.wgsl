@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn dpdy_699a05() -> vec4<f32> {
  var res : vec4<f32> = dpdy(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdy_699a05();
}
