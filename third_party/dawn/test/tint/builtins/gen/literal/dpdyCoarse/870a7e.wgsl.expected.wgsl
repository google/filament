@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn dpdyCoarse_870a7e() -> f32 {
  var res : f32 = dpdyCoarse(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdyCoarse_870a7e();
}
