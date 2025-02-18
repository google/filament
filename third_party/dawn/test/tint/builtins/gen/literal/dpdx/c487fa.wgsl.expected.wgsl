@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn dpdx_c487fa() -> vec4<f32> {
  var res : vec4<f32> = dpdx(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdx_c487fa();
}
