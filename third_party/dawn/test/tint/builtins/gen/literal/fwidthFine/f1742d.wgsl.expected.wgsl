@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn fwidthFine_f1742d() -> f32 {
  var res : f32 = fwidthFine(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fwidthFine_f1742d();
}
