@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn dpdxCoarse_029152() -> f32 {
  var res : f32 = dpdxCoarse(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdxCoarse_029152();
}
