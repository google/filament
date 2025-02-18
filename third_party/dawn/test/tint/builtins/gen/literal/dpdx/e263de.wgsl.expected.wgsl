@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn dpdx_e263de() -> f32 {
  var res : f32 = dpdx(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdx_e263de();
}
