@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn dpdy_feb40f() -> vec3<f32> {
  var res : vec3<f32> = dpdy(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdy_feb40f();
}
