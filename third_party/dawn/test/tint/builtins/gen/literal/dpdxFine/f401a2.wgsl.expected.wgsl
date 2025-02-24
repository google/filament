@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn dpdxFine_f401a2() -> f32 {
  var res : f32 = dpdxFine(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdxFine_f401a2();
}
