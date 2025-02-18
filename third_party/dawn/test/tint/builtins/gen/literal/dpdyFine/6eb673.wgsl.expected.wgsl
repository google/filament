@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn dpdyFine_6eb673() -> f32 {
  var res : f32 = dpdyFine(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdyFine_6eb673();
}
